#include "sys/app_controller.h"
#include "Arduino.h"
#include "common.h"
#include "sys/app_controller_gui.h"
#include "sys/interface.h"

// 文件名称限制在32个字符内
#define SYS_CONFIG_PATH "/mjt_sys.cfg"

// global
AppController *g_appController = NULL;          // APP控制器
volatile static bool g_timerHandleFlag = false; // imu数据更新标志

void TimerAppCtrlHandle(TimerHandle_t xTimer) { g_timerHandleFlag = true; }

void AppController::ReadConfigFromFlash(SysUtilConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char cfgInfo[MAX_CFG_INFO_LENGTH];
    int16_t size = g_flashFs.ReadFile(SYS_CONFIG_PATH, (uint8_t *)cfgInfo);
    if (size <= 0) {
        // 默认值
        cfg->ssid[0] = "Mate 50 Pro";
        cfg->password[0] = "12345678";
        cfg->powerMode = 0;             // 功耗模式（0为节能模式 1为性能模式）
        cfg->backlight = 80;            // 屏幕亮度（1-100）
        cfg->rotation = 4;              // 屏幕旋转方向
        cfg->imuAutoCalibration = 1;    // 是否自动校准陀螺仪 0关闭自动校准 1打开自动校准
        cfg->imuOrder = 0;              // 操作方向
        cfg->autoStartAppName = "None"; // 无指定开机自启APP
        this->WriteConfigToFlash(cfg);
    } else {
        cfgInfo[size] = 0;
        // 解析数据
        char *param[18] = {0};
        ParseParam(cfgInfo, 18, param);
        cfg->ssid[0] = param[0];
        cfg->password[0] = param[1];
        cfg->ssid[1] = param[2];
        cfg->password[1] = param[3];
        cfg->ssid[2] = param[4];
        cfg->password[2] = param[5];
        cfg->autoStartAppName = param[6]; // 开机自启APP的name
        cfg->powerMode = atol(param[7]);
        cfg->backlight = atol(param[8]);
        cfg->rotation = atol(param[9]);
        cfg->imuAutoCalibration = atol(param[10]);
        cfg->imuOrder = atol(param[11]);
        cfg->imuOffsets.imuGyroOffsetX = atol(param[12]);
        cfg->imuOffsets.imuGyroOffsetY = atol(param[13]);
        cfg->imuOffsets.imuGyroOffsetZ = atol(param[14]);
        cfg->imuOffsets.imuAccelOffsetX = atol(param[15]);
        cfg->imuOffsets.imuAccelOffsetY = atol(param[16]);
        cfg->imuOffsets.imuAccelOffsetZ = atol(param[17]);
    }
}

void AppController::WriteConfigToFlash(SysUtilConfig *cfg)
{
    char tmp[25];
    // 将配置数据保存在文件中（持久化）
    String wData;
    wData += cfg->ssid[0] + "\n";
    wData += cfg->password[0] + "\n";
    wData += cfg->ssid[1] + "\n";
    wData += cfg->password[1] + "\n";
    wData += cfg->ssid[2] + "\n";
    wData += cfg->password[2] + "\n";
    wData += cfg->autoStartAppName + "\n";
    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->powerMode);
    wData += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->backlight);
    wData += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->rotation);
    wData += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->imuAutoCalibration);
    wData += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->imuOrder);
    wData += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->imuOffsets.imuGyroOffsetX);
    wData += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->imuOffsets.imuGyroOffsetY);
    wData += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->imuOffsets.imuGyroOffsetZ);
    wData += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->imuOffsets.imuAccelOffsetX);
    wData += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->imuOffsets.imuAccelOffsetY);
    wData += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->imuOffsets.imuAccelOffsetZ);
    wData += tmp;

    g_flashFs.WriteFile(SYS_CONFIG_PATH, (const char *)wData.c_str());

    // 立即生效相关配置
    // screen.setBackLight(cfg->backlight / 100.0);
    // tft->setRotation(cfg->rotation);
    // m_imusetOrder(cfg->imuOrder);
}

void AppController::WifiRequestDeal(APP_MESSAGE_TYPE type)
{
    switch (type) {
        case APP_MESSAGE_WIFI_CONNECT:
            if (m_wifiStatus == WIFI_STATUS::WIFI_CONNECTED && (WiFi.getMode() & WIFI_MODE_AP)) {
                m_network.DisconnectWifi();
                m_wifiStatus = WIFI_STATUS::WIFI_DISCONNECTED;
            }
            if (m_wifiStatus == WIFI_STATUS::WIFI_DISCONNECTED) {
                m_wifiSsidItem = 0;
                m_network.ConnectWifi(m_sysCfg.ssid[m_wifiSsidItem].c_str(), m_sysCfg.password[m_wifiSsidItem].c_str());
                m_wifiStatus = WIFI_STATUS::WIFI_CONNECTING;
            } else if (m_wifiStatus == WIFI_STATUS::WIFI_CONNECTING && strcmp(m_requestFrom, CTRL_NAME) == 0) {
                m_network.ConnectWifi(m_sysCfg.ssid[m_wifiSsidItem].c_str(), m_sysCfg.password[m_wifiSsidItem].c_str());
            }
            m_preWifiReqMillis = millis();
            break;
        case APP_MESSAGE_WIFI_AP_START:
            if (m_wifiStatus == WIFI_STATUS::WIFI_CONNECTED && (WiFi.getMode() & WIFI_MODE_STA)) {
                m_network.DisconnectWifi();
                m_wifiStatus = WIFI_STATUS::WIFI_DISCONNECTED;
            }
            if (m_wifiStatus == WIFI_STATUS::WIFI_DISCONNECTED && m_network.OpenAp(AP_SSID)) {
                m_wifiStatus = WIFI_STATUS::WIFI_CONNECTED;
            }
            m_preWifiReqMillis = millis();
            break;
        case APP_MESSAGE_WIFI_DISCONNECT:
            if (m_wifiStatus != WIFI_STATUS::WIFI_DISCONNECTED) {
                m_network.DisconnectWifi();
                m_wifiStatus = WIFI_STATUS::WIFI_DISCONNECTED;
            }
            break;
        case APP_MESSAGE_WIFI_KEEP_ALIVE:
            if (m_wifiStatus != WIFI_STATUS::WIFI_DISCONNECTED) {
                m_preWifiReqMillis = millis();
            }
        default:
            break;
    }
}

void AppController::UpdateWifiStatus(void)
{
    if (m_wifiStatus == WIFI_STATUS::WIFI_DISCONNECTED)
        return;
    // 连接失败
    if (m_wifiStatus == WIFI_STATUS::WIFI_CONNECTING && (WiFi.getMode() & WIFI_MODE_STA) &&
        WiFi.status() != WL_CONNECTED && millis() - m_preWifiReqMillis >= 10000) {
        if (++m_wifiSsidItem < 3) {
            SendRequestEvent(CTRL_NAME, CTRL_NAME, APP_MESSAGE_WIFI_CONNECT, NULL, NULL);
            return;
        }
        m_wifiStatus = WIFI_STATUS::WIFI_DISCONNECTED;
        return;
    }
    if (m_wifiStatus == WIFI_STATUS::WIFI_CONNECTED && (WiFi.getMode() & WIFI_MODE_STA) &&
        WiFi.status() != WL_CONNECTED) {
        SendRequestEvent(CTRL_NAME, m_requestFrom, APP_MESSAGE_WIFI_DISCONNECT, NULL, NULL);
        m_wifiStatus = WIFI_STATUS::WIFI_DISCONNECTED;
        return;
    }
    if (m_wifiStatus == WIFI_STATUS::WIFI_CONNECTING && (WiFi.getMode() & WIFI_MODE_STA) &&
        WiFi.status() == WL_CONNECTED) {
        SendRequestEvent(CTRL_NAME, m_requestFrom, APP_MESSAGE_WIFI_CONNECTED, NULL, NULL);
        m_wifiStatus = WIFI_STATUS::WIFI_CONNECTED;
        return;
    }
    if (m_wifiStatus != WIFI_STATUS::WIFI_DISCONNECTED && DoDelayMillisTime(WIFI_LIFE_CYCLE, &m_preWifiReqMillis)) {
        Serial.println("\nwifi not in use, auto disconnect...");
        SendRequestEvent(CTRL_NAME, CTRL_NAME, APP_MESSAGE_WIFI_DISCONNECT, NULL, NULL);
        return;
    }
}

void AppController::GetParam(const char *key, char *value)
{
    if (!strcmp(key, "ssid0")) {
        snprintf(value, 32, "%s", m_sysCfg.ssid[0].c_str());
    } else if (!strcmp(key, "password0")) {
        snprintf(value, 32, "%s", m_sysCfg.password[0].c_str());
    } else if (!strcmp(key, "ssid1")) {
        snprintf(value, 32, "%s", m_sysCfg.ssid[1].c_str());
    } else if (!strcmp(key, "password1")) {
        snprintf(value, 32, "%s", m_sysCfg.password[1].c_str());
    } else if (!strcmp(key, "ssid2")) {
        snprintf(value, 32, "%s", m_sysCfg.ssid[2].c_str());
    } else if (!strcmp(key, "password2")) {
        snprintf(value, 32, "%s", m_sysCfg.password[2].c_str());
    } else if (!strcmp(key, "autoStartAppName")) {
        snprintf(value, 32, "%s", m_sysCfg.autoStartAppName.c_str());
    } else if (!strcmp(key, "powerMode")) {
        snprintf(value, 32, "%u", m_sysCfg.powerMode);
    } else if (!strcmp(key, "backlight")) {
        snprintf(value, 32, "%u", m_sysCfg.backlight);
    } else if (!strcmp(key, "rotation")) {
        snprintf(value, 32, "%u", m_sysCfg.rotation);
    } else if (!strcmp(key, "imuAutoCalibration")) {
        snprintf(value, 32, "%u", m_sysCfg.imuAutoCalibration);
    } else if (!strcmp(key, "imuOrder")) {
        snprintf(value, 32, "%u", m_sysCfg.imuOrder);
    }
}

void AppController::SetParam(const char *key, const char *value)
{
    if (!strcmp(key, "ssid0")) {
        m_sysCfg.ssid[0] = value;
    } else if (!strcmp(key, "password0")) {
        m_sysCfg.password[0] = value;
    } else if (!strcmp(key, "ssid1")) {
        m_sysCfg.ssid[1] = value;
    } else if (!strcmp(key, "password1")) {
        m_sysCfg.password[1] = value;
    } else if (!strcmp(key, "ssid2")) {
        m_sysCfg.ssid[2] = value;
    } else if (!strcmp(key, "password2")) {
        m_sysCfg.password[2] = value;
    } else if (!strcmp(key, "imuAutoCalibration")) {
        m_sysCfg.imuAutoCalibration = atol(value);
    } else if (!strcmp(key, "powerMode")) {
        m_sysCfg.powerMode = atol(value);
    } else if (!strcmp(key, "backlight")) {
        m_sysCfg.backlight = atol(value);
    } else if (!strcmp(key, "rotation")) {
        m_sysCfg.rotation = atol(value);
    } else if (!strcmp(key, "imuOrder")) {
        m_sysCfg.imuOrder = atol(value);
    } else if (!strcmp(key, "autoStartAppName")) {
        m_sysCfg.autoStartAppName = value;
    }
}

void AppController::RequestProcess(APP_MESSAGE_TYPE type, void *data, void *extData)
{
    switch (type) {
        case APP_MESSAGE_WIFI_CONNECT:
        case APP_MESSAGE_WIFI_AP_START:
        case APP_MESSAGE_WIFI_DISCONNECT:
        case APP_MESSAGE_WIFI_KEEP_ALIVE:
            WifiRequestDeal(type);
            break;
        case APP_MESSAGE_GET_PARAM:
            GetParam((const char *)data, (char *)extData);
            break;
        case APP_MESSAGE_SET_PARAM:
            SetParam((const char *)data, (const char *)extData);
            break;
        case APP_MESSAGE_READ_CFG:
            ReadConfigFromFlash(&m_sysCfg);
            break;
        case APP_MESSAGE_WRITE_CFG:
            WriteConfigToFlash(&m_sysCfg);
            break;
        default:
            break;
    }
}

AppController::AppController(const char *name)
{
    m_name = name;
    m_appNum = 0;
    m_currentAppItem = 0;
    m_wifiSsidItem = 0;
    m_wifiStatus = WIFI_STATUS::WIFI_DISCONNECTED;
    m_preWifiReqMillis = millis();
    m_appCtrlState = MJT_SYS_STATE::STATE_SYS_LOADING;
    m_imuActionData = NULL;
    m_requestFrom = NULL;
    // 定义一个定时器
    m_appCtrlTimer = xTimerCreate("AppCtrlTimer", 200 / portTICK_PERIOD_MS, pdTRUE, (void *)0, TimerAppCtrlHandle);
}

AppController::~AppController() {}

void AppController::Init(void)
{
    // FlashFs init first
    g_flashFs.Init();

    this->ReadConfigFromFlash(&m_sysCfg);

    // 设置CPU主频
    if (1 == m_sysCfg.powerMode) {
        setCpuFrequencyMhz(240);
    } else {
        setCpuFrequencyMhz(80);
    }
    Serial.print(F("PowerMode: "));
    Serial.print(m_sysCfg.powerMode);
    Serial.print(F(", CpuFrequencyMhz: "));
    Serial.println(getCpuFrequencyMhz());

    /*** Init micro SD-Card ***/
    g_tfCard.Init();

    /*** Init screen ***/
    tft = new TFT_eSPI(SCREEN_HOR_RES, SCREEN_VER_RES);
    screen.init(m_sysCfg.rotation, m_sysCfg.backlight);

    InitLvglTaskSetup("LvglTask");

    MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingGuiInit());
    MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingDisplay(10, NULL, true));

    /*** Init IMU as input device ***/
    // lv_port_indev_init();

    MJT_LVGL_OPERATE_LOCK(AppCtrlLoadingDisplay(80, "init imu...", false));
    m_imu.Init(m_sysCfg.imuOrder, m_sysCfg.imuAutoCalibration, &m_sysCfg.imuOffsets); // 初始化比较耗时

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
    DeleteLvglTask();
    MJT_LVGL_OPERATE_LOCK(AppCtrlMenuGuiInit());
    MJT_LVGL_OPERATE_LOCK(AppCtrlMenuDisplay(m_appList[m_currentAppItem]->appLogo, m_appList[m_currentAppItem]->appName,
                                             LV_SCR_LOAD_ANIM_FADE_IN, true));

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

    m_appList.push_back(app);
    ++m_appNum;
    return 0; // 安装成功
}

int AppController::AppAutoStart(void)
{
    // APP自启动
    int index = this->GetAppIndexByName(m_sysCfg.autoStartAppName.c_str());
    if (index < 0) {
        // 没找到相关的APP
        return 0;
    }
    // 进入自启动的APP
    SetSystemState(MJT_SYS_STATE::STATE_APP_RUNNING);
    m_currentAppItem = index;
    (*(m_appList[m_currentAppItem]->AppInit))(this); // 执行APP初始化
    return 0;
}

void AppController::MainProcess(void)
{
    MJT_LVGL_OPERATE_LOCK(lv_timer_handler());

    if (unlikely(GetSystemState() == MJT_SYS_STATE::STATE_SYS_LOADING)) {
        delay(1);
        return;
    }

    if (g_timerHandleFlag) {
        g_timerHandleFlag = false;
        m_imuActionData = m_imu.getAction();
        UpdateWifiStatus();
    }

    if (ACTIVE_TYPE::UNKNOWN != m_imuActionData->active && ACTIVE_TYPE::SHAKE != m_imuActionData->active) {
        Serial.print("[Operate] ");
        Serial.println(active_type_info[m_imuActionData->active]);
    }

    if (GetSystemState() == MJT_SYS_STATE::STATE_APP_MENU) {
        // lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_NONE;

        if (m_imuActionData->active == ACTIVE_TYPE::TURN_LEFT) {
            m_currentAppItem = (m_currentAppItem + 1) % m_appNum;
            AppCtrlMenuDisplay(m_appList[m_currentAppItem]->appLogo, m_appList[m_currentAppItem]->appName,
                               LV_SCR_LOAD_ANIM_MOVE_LEFT, false);
            Serial.println(String("Current App: ") + m_appList[m_currentAppItem]->appName);
        } else if (m_imuActionData->active == ACTIVE_TYPE::TURN_RIGHT) {
            m_currentAppItem = (m_currentAppItem + m_appNum - 1) % m_appNum;
            AppCtrlMenuDisplay(m_appList[m_currentAppItem]->appLogo, m_appList[m_currentAppItem]->appName,
                               LV_SCR_LOAD_ANIM_MOVE_RIGHT, false);
            Serial.println(String("Current App: ") + m_appList[m_currentAppItem]->appName);
        } else if (m_imuActionData->active == ACTIVE_TYPE::GO_FORWORD) {
            if (m_appList[m_currentAppItem]->AppInit != NULL) {
                (*(m_appList[m_currentAppItem]->AppInit))(this); // 执行APP初始化
                SetSystemState(MJT_SYS_STATE::STATE_APP_RUNNING);
            }
        }
    } else if (GetSystemState() == MJT_SYS_STATE::STATE_APP_RUNNING) {
        (*(m_appList[m_currentAppItem]->MainProcess))(this, m_imuActionData);
    }

    m_imuActionData->active = ACTIVE_TYPE::UNKNOWN;
    m_imuActionData->isValid = 0;
}

APP_OBJ *AppController::GetAppByName(const char *name)
{
    for (APP_OBJ *app : m_appList) {
        if (!strcmp(name, app->appName)) {
            return app;
        }
    }
    return NULL;
}

int AppController::GetAppIndexByName(const char *name)
{
    for (int pos = 0; pos < m_appNum; ++pos) {
        if (!strcmp(name, m_appList[pos]->appName)) {
            return pos;
        }
    }

    return -1;
}

void AppController::AppExit(void)
{
    lv_anim_del_all();
    AppCtrlMenuDisplay(m_appList[m_currentAppItem]->appLogo, m_appList[m_currentAppItem]->appName,
                       LV_SCR_LOAD_ANIM_FADE_IN, false);

    if (NULL != m_appList[m_currentAppItem]->AppExit) {
        // 执行APP退出回调
        (*(m_appList[m_currentAppItem]->AppExit))(NULL);
    }

    // 设置CPU主频
    if (1 == this->m_sysCfg.powerMode) {
        setCpuFrequencyMhz(240);
    } else {
        setCpuFrequencyMhz(80);
    }
    Serial.print(F("CpuFrequencyMhz: "));
    Serial.println(getCpuFrequencyMhz());

    SetSystemState(MJT_SYS_STATE::STATE_APP_MENU);
}