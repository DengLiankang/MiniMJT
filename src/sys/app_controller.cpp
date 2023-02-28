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
AppController *g_appController = NULL;         // APP控制器
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
    m_appNum = 0;
    app_exit_flag = 0;
    mCurrentAppItem = 0;
    pre_app_index = 0;
    // m_appList = new APP_OBJ[APP_MAX_NUM];
    m_wifi_status = false;
    m_preWifiReqMillis = GET_SYS_MILLIS();
    m_appCtrlState = MJT_SYS_STATE::STATE_SYS_LOADING;
    // 定义一个定时器
    m_appCtrlTimer = xTimerCreate("AppCtrlTimer", 200 / portTICK_PERIOD_MS, pdTRUE, (void *)0, TimerAppCtrlHandle);
}

AppController::~AppController() {}

void AppController::Init(void)
{
    // flashfs init first
    gFlashCfg.Init();

    this->ReadConfig(&m_sysCfg);
    this->ReadConfig(&m_imuCfg);

    // 设置CPU主频
    if (1 == m_sysCfg.power_mode) {
        setCpuFrequencyMhz(240);
    } else {
        setCpuFrequencyMhz(80);
    }
    Serial.print(F("PowerMode: "));
    Serial.print(m_sysCfg.power_mode);
    Serial.print(F(", CpuFrequencyMhz: "));
    Serial.println(getCpuFrequencyMhz());

    /*** Init screen ***/
    screen.init(m_sysCfg.rotation, m_sysCfg.backLight);

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
    mpu.init(m_sysCfg.mpu_order, m_sysCfg.auto_calibration_mpu, &m_imuCfg); // 初始化比较耗时

    MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingDisplay(90, "install apps...", false));

    // 启动事件处理定时器
    xTimerStart(m_appCtrlTimer, 500 / portTICK_PERIOD_MS);
}

void AppController::ExitLoadingGui(void)
{
    if (!m_appNum) {
        MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingDisplay(100, "#ff0000 " LV_SYMBOL_WARNING "# no app!", true));
        return;
    }
    MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingDisplay(100, "finished.", true));
    MJT_LVGL_OPERATE_LOCK(AppCtrlMenuGuiInit());
    MJT_LVGL_OPERATE_LOCK(AppCtrlMunuDisplay(m_appList[mCurrentAppItem]->appLogo, m_appList[mCurrentAppItem]->appName, LV_SCR_LOAD_ANIM_FADE_IN, true));

    SetSystemState(STATE_APP_MENU);
}

MJT_SYS_STATE AppController::GetSystemState(void) { return m_appCtrlState; }

void AppController::SetSystemState(MJT_SYS_STATE state) { m_appCtrlState = state; }

int AppController::AppIsLegal(const APP_OBJ *appObj)
{
    // APP的合法性检测
    if (NULL == appObj)
        return -1;
    if (APP_MAX_NUM <= m_appNum)
        return -2;
    for (int pos = 0; pos < m_appNum; ++pos) {
        if (!strcmp(appObj->appName, m_appList[pos]->appName)) {
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

    m_appList[m_appNum] = app;
    m_appTypeList[m_appNum] = appType;
    ++m_appNum;
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
    int index = this->getAppIdxByName(m_sysCfg.auto_start_app.c_str());
    if (index < 0) {
        // 没找到相关的APP
        return 0;
    }
    // 进入自启动的APP
    app_exit_flag = 1; // 进入app, 如果已经在
    mCurrentAppItem = index;
    (*(m_appList[mCurrentAppItem]->appInit))(this); // 执行APP初始化
    return 0;
}

void AppController::MainProcess(void)
{
    MJT_LVGL_OPERATE_LOCK(lv_timer_handler());

    if (unlikely(g_appController->GetSystemState() == MJT_SYS_STATE::STATE_SYS_LOADING)) {
        delay(1);
        return;
    }

    // 有动画刷新时不进行处理
    // if (lv_anim_count_running()) {
    //     delay(1);
    //     return;
    // }

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
    // if (0 == m_sysCfg.power_mode && true == m_wifi_status &&
    //     doDelayMillisTime(WIFI_LIFE_CYCLE, &m_preWifiReqMillis, false)) {
    //     send_to(CTRL_NAME, CTRL_NAME, APP_MESSAGE_WIFI_DISCONN, 0, NULL);
    // }

    if (g_appController->GetSystemState() == MJT_SYS_STATE::STATE_APP_MENU) {
        // lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_NONE;

        if (gImuActionData->active == ACTIVE_TYPE::TURN_LEFT) {
            mCurrentAppItem = (mCurrentAppItem + 1) % m_appNum;
            AppCtrlMunuDisplay(m_appList[mCurrentAppItem]->appLogo, m_appList[mCurrentAppItem]->appName, LV_SCR_LOAD_ANIM_MOVE_LEFT, false);
            Serial.println(String("Current App: ") + m_appList[mCurrentAppItem]->appName);
            ANIEND_WAIT;
        } else if (gImuActionData->active == ACTIVE_TYPE::TURN_RIGHT) {
            mCurrentAppItem = (mCurrentAppItem + m_appNum - 1) % m_appNum;
            AppCtrlMunuDisplay(m_appList[mCurrentAppItem]->appLogo, m_appList[mCurrentAppItem]->appName, LV_SCR_LOAD_ANIM_MOVE_RIGHT, false);
            Serial.println(String("Current App: ") + m_appList[mCurrentAppItem]->appName);
            ANIEND_WAIT;
        } else if (gImuActionData->active == ACTIVE_TYPE::GO_FORWORD) {
            if (m_appList[mCurrentAppItem]->appInit != NULL) {
                (*(m_appList[mCurrentAppItem]->appInit))(this); // 执行APP初始化
                g_appController->SetSystemState(MJT_SYS_STATE::STATE_APP_RUNNING);
            }
        }
    } else if (g_appController->GetSystemState() == MJT_SYS_STATE::STATE_APP_RUNNING) {
        (*(m_appList[mCurrentAppItem]->MainProcess))(this, gImuActionData);
    }

    // if (0 == app_exit_flag) {
    //     // 当前没有进入任何app
    //     lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
    //     if (ACTIVE_TYPE::TURN_LEFT == gImuActionData->active) {
    //         anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
    //         pre_app_index = mCurrentAppItem;
    //         mCurrentAppItem = (mCurrentAppItem + 1) % m_appNum;
    //         Serial.println(String("Current App: ") + m_appList[mCurrentAppItem]->appName);
    //     } else if (ACTIVE_TYPE::TURN_RIGHT == gImuActionData->active) {
    //         anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
    //         pre_app_index = mCurrentAppItem;
    //         // 以下等效与 processId = (processId - 1 + m_appNum) % 4;
    //         // +3为了不让数据溢出成负数，而导致取模逻辑错误
    //         mCurrentAppItem = (mCurrentAppItem - 1 + m_appNum) % m_appNum; // 此处的3与p_processList的长度一致
    //         Serial.println(String("Current App: ") + m_appList[mCurrentAppItem]->appName);
    //     } else if (ACTIVE_TYPE::GO_FORWORD == gImuActionData->active) {
    //         app_exit_flag = 1; // 进入app
    //         if (NULL != m_appList[mCurrentAppItem]->appInit) {
    //             (*(m_appList[mCurrentAppItem]->appInit))(this); // 执行APP初始化
    //         }
    //     }

    //     if (ACTIVE_TYPE::GO_FORWORD != gImuActionData->active) // && UNKNOWN != gImuActionData->active
    //     {
    //         app_control_display_scr(m_appList[mCurrentAppItem]->appLogo, m_appList[mCurrentAppItem]->appName, anim_type,
    //                                 false);
    //         vTaskDelay(200 / portTICK_PERIOD_MS);
    //     }
    // } else {
    //     app_control_display_scr(m_appList[mCurrentAppItem]->appLogo, m_appList[mCurrentAppItem]->appName,
    //                             LV_SCR_LOAD_ANIM_NONE, false);
    //     // 运行APP进程 等效于把控制权交给当前APP
    //     (*(m_appList[mCurrentAppItem]->MainProcess))(this, gImuActionData);
    // }
    gImuActionData->active = ACTIVE_TYPE::UNKNOWN;
    gImuActionData->isValid = 0;
}

APP_OBJ *AppController::getAppByName(const char *name)
{
    for (int pos = 0; pos < m_appNum; ++pos) {
        if (!strcmp(name, m_appList[pos]->appName)) {
            return m_appList[pos];
        }
    }

    return NULL;
}

int AppController::getAppIdxByName(const char *name)
{
    for (int pos = 0; pos < m_appNum; ++pos) {
        if (!strcmp(name, m_appList[pos]->appName)) {
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
            Serial.print("[Massage]\tFrom " + String(fromApp->appName) + "\tTo " + String(toApp->appName) + "\n");
            if (NULL != toApp->messageHandle) {
                toApp->messageHandle(from, to, type, message, ext_info);
            }
        } else if (!strcmp(to, CTRL_NAME)) {
            Serial.print("[Massage]\tFrom " + String(fromApp->appName) + "\tTo " + CTRL_NAME + "\n");
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
        if (NULL != (*event).from && NULL != (*event).from->messageHandle) {
            (*((*event).from->messageHandle))(CTRL_NAME, (*event).from->appName, (*event).type, (*event).info, NULL);
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
                g_network.start_conn_wifi(m_sysCfg.ssid_0.c_str(), m_sysCfg.password_0.c_str());
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
            if (app_exit_flag == 1 && mCurrentAppItem != getAppIdxByName("Heartbeat")) // 在其他app中
            {
                app_exit_flag = 0;
                (*(m_appList[mCurrentAppItem]->appExit))(NULL); // 退出当前app
            }
            if (app_exit_flag == 0) {
                app_exit_flag = 1; // 进入app, 如果已经在
                mCurrentAppItem = getAppIdxByName("Heartbeat");
                (*(getAppByName("Heartbeat")->appInit))(this); // 执行APP初始化
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
        if (m_appList[mCurrentAppItem] == (*event).from) {
            event = eventList.erase(event); // 删除该响应事件
        } else {
            ++event;
        }
    }

    if (NULL != m_appList[mCurrentAppItem]->appExit) {
        // 执行APP退出回调
        (*(m_appList[mCurrentAppItem]->appExit))(NULL);
    }
    // app_control_display_scr(m_appList[mCurrentAppItem]->appLogo, m_appList[mCurrentAppItem]->appName,
    //                         LV_SCR_LOAD_ANIM_NONE, true);

    // 设置CPU主频
    if (1 == this->m_sysCfg.power_mode) {
        setCpuFrequencyMhz(240);
    } else {
        setCpuFrequencyMhz(80);
    }
    Serial.print(F("CpuFrequencyMhz: "));
    Serial.println(getCpuFrequencyMhz());

    SetSystemState(MJT_SYS_STATE::STATE_APP_MENU);
}