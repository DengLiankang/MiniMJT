#include "sys/app_controller.h"
#include "Arduino.h"
#include "common.h"
#include "driver/lv_port_fs.h"
#include "sys/app_controller_gui.h"
#include "sys/interface.h"

const char *app_event_type_info[] = {"APP_MESSAGE_WIFI_CONN",    "APP_MESSAGE_WIFI_AP",     "APP_MESSAGE_WIFI_ALIVE",
                                     "APP_MESSAGE_WIFI_DISCONN", "APP_MESSAGE_UPDATE_TIME", "APP_MESSAGE_MQTT_DATA",
                                     "APP_MESSAGE_GET_PARAM",    "APP_MESSAGE_SET_PARAM",   "APP_MESSAGE_READ_CFG",
                                     "APP_MESSAGE_WRITE_CFG",    "APP_MESSAGE_NONE"};

// global
AppController *gAppController = NULL;         // APP控制器
volatile static bool gIsRunEventDeal = false; // 事件处理标志
volatile static bool gIsCheckAction = false;  // imu数据更新标志
ImuAction *gImuActionData = NULL;             // 存放mpu6050数据

void TimerAppCtrlHandle(TimerHandle_t xTimer)
{
    gIsRunEventDeal = true;
    gIsCheckAction = true;
}

AppController::AppController(const char *name)
{
    strncpy(this->name, name, APP_CONTROLLER_NAME_LEN);
    mAppNum = 0;
    app_exit_flag = 0;
    cur_app_index = 0;
    pre_app_index = 0;
    // mAppList = new APP_OBJ[APP_MAX_NUM];
    m_wifi_status = false;
    m_preWifiReqMillis = GET_SYS_MILLIS();
    mAppCtrlState = MJT_SYS_STATE::STATE_SYS_LOADING;
    // 定义一个定时器
    mTimerAppCtrl = xTimerCreate("AppCtrlTimer", 200 / portTICK_PERIOD_MS, pdTRUE, (void *)0, TimerAppCtrlHandle);
}

AppController::~AppController() {}

void AppController::Init(void)
{
    // flashfs init first
    gFlashCfg.Init();

    this->ReadConfig(&mSysCfg);
    this->ReadConfig(&mImuCfg);

    // 设置CPU主频
    if (1 == mSysCfg.power_mode) {
        setCpuFrequencyMhz(240);
    } else {
        setCpuFrequencyMhz(80);
    }
    Serial.print(F("PowerMode: "));
    Serial.print(mSysCfg.power_mode);
    Serial.print(F(", CpuFrequencyMhz: "));
    Serial.println(getCpuFrequencyMhz());

    /*** Init screen ***/
    screen.init(mSysCfg.rotation, mSysCfg.backLight);

    MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingGuiInit());
    MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingDisplay(10, NULL, true));
    /*** Init ambient-light sensor ***/
    ambLight.init(ONE_TIME_H_RESOLUTION_MODE);

    /*** Init micro SD-Card ***/
    tf.init();

    lv_fs_fatfs_init();

    /*** Init IMU as input device ***/
    // lv_port_indev_init();

    MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingDisplay(80, "init imu...", false));
    mpu.init(mSysCfg.mpu_order, mSysCfg.auto_calibration_mpu, &mImuCfg); // 初始化比较耗时

    MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingDisplay(90, "install apps...", false));
    // mAppList[0] = new APP_OBJ();
    // mAppList[0]->app_image = &app_loading;
    // mAppList[0]->app_name = "Loading...";

    // app_control_display_scr(mAppList[cur_app_index]->app_image, mAppList[cur_app_index]->app_name,
    //                         LV_SCR_LOAD_ANIM_NONE, false);
    // 启动事件处理定时器
    xTimerStart(mTimerAppCtrl, 0);
}

void AppController::ExitLoadingGui(void)
{
    if (!mAppNum) {
        MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingDisplay(100, "#ff0000 " LV_SYMBOL_WARNING "# no app!", true));
        return;
    }
    MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingDisplay(100, "finished.", true));

    MJT_LVGL_OPERATE_LOCK(AppCtrlMenuGuiInit());
    MJT_LVGL_OPERATE_LOCK(AppCtrlMunuDisplay(mAppList[cur_app_index]->app_image, mAppList[cur_app_index]->app_name, LV_SCR_LOAD_ANIM_FADE_IN, true));

    SetSystemState(STATE_APP_MENU);
}

MJT_SYS_STATE AppController::GetSystemState(void) { return mAppCtrlState; }

void AppController::SetSystemState(MJT_SYS_STATE state) { mAppCtrlState = state; }

int AppController::AppIsLegal(const APP_OBJ *appObj)
{
    // APP的合法性检测
    if (NULL == appObj)
        return -1;
    if (APP_MAX_NUM <= mAppNum)
        return -2;
    for (int pos = 0; pos < mAppNum; ++pos) {
        if (!strcmp(appObj->app_name, mAppList[pos]->app_name)) {
            return -3;
        }
    }
    return 0;
}

// 将APP安装到app_controller中
int AppController::AppInstall(APP_OBJ *app, APP_TYPE appType)
{
    int ret = AppIsLegal(app);
    if (0 != ret) {
        return ret;
    }

    mAppList[mAppNum] = app;
    appTypeList[mAppNum] = appType;
    ++mAppNum;
    return 0; // 安装成功
}

// 将APP的后台任务从任务队列中移除(自能通过APP退出的时候，移除自身的后台任务)
int AppController::remove_backgroud_task(void)
{
    return 0; // 安装成功
}

int AppController::app_auto_start()
{
    // APP自启动
    int index = this->getAppIdxByName(mSysCfg.auto_start_app.c_str());
    if (index < 0) {
        // 没找到相关的APP
        return 0;
    }
    // 进入自启动的APP
    app_exit_flag = 1; // 进入app, 如果已经在
    cur_app_index = index;
    (*(mAppList[cur_app_index]->app_init))(this); // 执行APP初始化
    return 0;
}

int AppController::MainProcess(void)
{
    MJT_LVGL_OPERATE_LOCK(lv_timer_handler());

    if (gIsCheckAction) {
        gIsCheckAction = false;
        gImuActionData = mpu.getAction();
    }

    if (gIsRunEventDeal) {
        gIsRunEventDeal = false;
        this->req_event_deal();
    }

    if (ACTIVE_TYPE::UNKNOWN != gImuActionData->active) {
        Serial.print("[Operate] ");
        Serial.println(active_type_info[gImuActionData->active]);
    }

    // // wifi自动关闭(在节能模式下)
    // if (0 == mSysCfg.power_mode && true == m_wifi_status &&
    //     doDelayMillisTime(WIFI_LIFE_CYCLE, &m_preWifiReqMillis, false)) {
    //     send_to(CTRL_NAME, CTRL_NAME, APP_MESSAGE_WIFI_DISCONN, 0, NULL);
    // }

    // if (0 == app_exit_flag) {
    //     // 当前没有进入任何app
    //     lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
    //     if (ACTIVE_TYPE::TURN_LEFT == gImuActionData->active) {
    //         anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
    //         pre_app_index = cur_app_index;
    //         cur_app_index = (cur_app_index + 1) % mAppNum;
    //         Serial.println(String("Current App: ") + mAppList[cur_app_index]->app_name);
    //     } else if (ACTIVE_TYPE::TURN_RIGHT == gImuActionData->active) {
    //         anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
    //         pre_app_index = cur_app_index;
    //         // 以下等效与 processId = (processId - 1 + mAppNum) % 4;
    //         // +3为了不让数据溢出成负数，而导致取模逻辑错误
    //         cur_app_index = (cur_app_index - 1 + mAppNum) % mAppNum; // 此处的3与p_processList的长度一致
    //         Serial.println(String("Current App: ") + mAppList[cur_app_index]->app_name);
    //     } else if (ACTIVE_TYPE::GO_FORWORD == gImuActionData->active) {
    //         app_exit_flag = 1; // 进入app
    //         if (NULL != mAppList[cur_app_index]->app_init) {
    //             (*(mAppList[cur_app_index]->app_init))(this); // 执行APP初始化
    //         }
    //     }

    //     if (ACTIVE_TYPE::GO_FORWORD != gImuActionData->active) // && UNKNOWN != gImuActionData->active
    //     {
    //         app_control_display_scr(mAppList[cur_app_index]->app_image, mAppList[cur_app_index]->app_name, anim_type,
    //                                 false);
    //         vTaskDelay(200 / portTICK_PERIOD_MS);
    //     }
    // } else {
    //     app_control_display_scr(mAppList[cur_app_index]->app_image, mAppList[cur_app_index]->app_name,
    //                             LV_SCR_LOAD_ANIM_NONE, false);
    //     // 运行APP进程 等效于把控制权交给当前APP
    //     (*(mAppList[cur_app_index]->MainProcess))(this, gImuActionData);
    // }
    gImuActionData->active = ACTIVE_TYPE::UNKNOWN;
    gImuActionData->isValid = 0;
    return 0;
}

APP_OBJ *AppController::getAppByName(const char *name)
{
    for (int pos = 0; pos < mAppNum; ++pos) {
        if (!strcmp(name, mAppList[pos]->app_name)) {
            return mAppList[pos];
        }
    }

    return NULL;
}

int AppController::getAppIdxByName(const char *name)
{
    for (int pos = 0; pos < mAppNum; ++pos) {
        if (!strcmp(name, mAppList[pos]->app_name)) {
            return pos;
        }
    }

    return -1;
}

// 通信中心（消息转发）
int AppController::send_to(const char *from, const char *to, APP_MESSAGE_TYPE type, void *message, void *ext_info)
{
    APP_OBJ *fromApp = getAppByName(from); // 来自谁 有可能为空
    APP_OBJ *toApp = getAppByName(to);     // 发送给谁 有可能为空
    if (type <= APP_MESSAGE_MQTT_DATA) {
        // 更新事件的请求者
        if (eventList.size() > EVENT_LIST_MAX_LENGTH) {
            return 1;
        }
        // 发给控制器的消息(目前都是wifi事件)
        EVENT_OBJ new_event = {fromApp, type, message, 3, 0, 0};
        eventList.push_back(new_event);
        Serial.print("[EVENT]\tAdd -> " + String(app_event_type_info[type]));
        Serial.print(F("\tEventList Size: "));
        Serial.println(eventList.size());
    } else {
        // 各个APP之间通信的消息
        if (NULL != toApp) {
            Serial.print("[Massage]\tFrom " + String(fromApp->app_name) + "\tTo " + String(toApp->app_name) + "\n");
            if (NULL != toApp->message_handle) {
                toApp->message_handle(from, to, type, message, ext_info);
            }
        } else if (!strcmp(to, CTRL_NAME)) {
            Serial.print("[Massage]\tFrom " + String(fromApp->app_name) + "\tTo " + CTRL_NAME + "\n");
            deal_config(type, (const char *)message, (char *)ext_info);
        }
    }
    return 0;
}

int AppController::req_event_deal(void)
{
    // 请求事件的处理
    for (std::list<EVENT_OBJ>::iterator event = eventList.begin(); event != eventList.end();) {
        if ((*event).nextRunTime > GET_SYS_MILLIS()) {
            ++event;
            continue;
        }
        // 后期可以拓展其他事件的处理
        bool ret = wifi_event((*event).type);
        if (false == ret) {
            // 本事件没处理完成
            (*event).retryCount += 1;
            if ((*event).retryCount >= (*event).retryMaxNum) {
                // 多次重试失败
                Serial.print("[EVENT]\tDelete -> " + String(app_event_type_info[(*event).type]));
                event = eventList.erase(event); // 删除该响应事件
                Serial.print(F("\tEventList Size: "));
                Serial.println(eventList.size());
            } else {
                // 下次重试
                (*event).nextRunTime = GET_SYS_MILLIS() + 4000;
                ++event;
            }
            continue;
        }

        // 事件回调
        if (NULL != (*event).from && NULL != (*event).from->message_handle) {
            (*((*event).from->message_handle))(CTRL_NAME, (*event).from->app_name, (*event).type, (*event).info, NULL);
        }
        Serial.print("[EVENT]\tDelete -> " + String(app_event_type_info[(*event).type]));
        event = eventList.erase(event); // 删除该响应完成的事件
        Serial.print(F("\tEventList Size: "));
        Serial.println(eventList.size());
    }
    return 0;
}

/**
 *  wifi事件的处理
 *  事件处理成功返回true 否则false
 * */
bool AppController::wifi_event(APP_MESSAGE_TYPE type)
{
    switch (type) {
        case APP_MESSAGE_WIFI_CONN: {
            // 更新请求
            // CONN_ERROR == g_network.end_conn_wifi() ||
            if (false == m_wifi_status) {
                g_network.start_conn_wifi(mSysCfg.ssid_0.c_str(), mSysCfg.password_0.c_str());
                m_wifi_status = true;
            }
            m_preWifiReqMillis = GET_SYS_MILLIS();
            if ((WiFi.getMode() & WIFI_MODE_STA) == WIFI_MODE_STA && CONN_SUCC != g_network.end_conn_wifi()) {
                // 在STA模式下 并且还没连接上wifi
                return false;
            }
        } break;
        case APP_MESSAGE_WIFI_AP: {
            // 更新请求
            g_network.open_ap(AP_SSID);
            m_wifi_status = true;
            m_preWifiReqMillis = GET_SYS_MILLIS();
        } break;
        case APP_MESSAGE_WIFI_ALIVE: {
            // wifi开关的心跳 持续收到心跳 wifi才不会被关闭
            m_wifi_status = true;
            // 更新请求
            m_preWifiReqMillis = GET_SYS_MILLIS();
        } break;
        case APP_MESSAGE_WIFI_DISCONN: {
            g_network.close_wifi();
            m_wifi_status = false; // 标志位
            // m_preWifiReqMillis = GET_SYS_MILLIS() - WIFI_LIFE_CYCLE;
        } break;
        case APP_MESSAGE_UPDATE_TIME: {
        } break;
        case APP_MESSAGE_MQTT_DATA: {
            Serial.println("APP_MESSAGE_MQTT_DATA");
            if (app_exit_flag == 1 && cur_app_index != getAppIdxByName("Heartbeat")) // 在其他app中
            {
                app_exit_flag = 0;
                (*(mAppList[cur_app_index]->exit_callback))(NULL); // 退出当前app
            }
            if (app_exit_flag == 0) {
                app_exit_flag = 1; // 进入app, 如果已经在
                cur_app_index = getAppIdxByName("Heartbeat");
                (*(getAppByName("Heartbeat")->app_init))(this); // 执行APP初始化
            }
        } break;
        default:
            break;
    }

    return true;
}

void AppController::app_exit()
{
    app_exit_flag = 0; // 退出APP

    // 清空该对象的所有请求
    for (std::list<EVENT_OBJ>::iterator event = eventList.begin(); event != eventList.end();) {
        if (mAppList[cur_app_index] == (*event).from) {
            event = eventList.erase(event); // 删除该响应事件
        } else {
            ++event;
        }
    }

    if (NULL != mAppList[cur_app_index]->exit_callback) {
        // 执行APP退出回调
        (*(mAppList[cur_app_index]->exit_callback))(NULL);
    }
    app_control_display_scr(mAppList[cur_app_index]->app_image, mAppList[cur_app_index]->app_name,
                            LV_SCR_LOAD_ANIM_NONE, true);

    // 设置CPU主频
    if (1 == this->mSysCfg.power_mode) {
        setCpuFrequencyMhz(240);
    } else {
        setCpuFrequencyMhz(80);
    }
    Serial.print(F("CpuFrequencyMhz: "));
    Serial.println(getCpuFrequencyMhz());
}