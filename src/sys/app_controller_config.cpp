#include "Arduino.h"
#include "sys/app_controller.h"

#define SYS_CONFIG_PATH "/minimjt_sys.cfg"

void AppController::ReadConfigFromFlash(SysUtilConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char cfgInfo[MAX_CFG_INFO_LENGTH];
    uint16_t size = g_flashFs.ReadFile(SYS_CONFIG_PATH, (uint8_t *)cfgInfo);
    cfgInfo[size] = 0;
    if (size == 0) {
        // 默认值
        cfg->powerMode = 0;             // 功耗模式（0为节能模式 1为性能模式）
        cfg->backlight = 80;            // 屏幕亮度（1-100）
        cfg->rotation = 4;              // 屏幕旋转方向
        cfg->imuAutoCalibration = 1;    // 是否自动校准陀螺仪 0关闭自动校准 1打开自动校准
        cfg->imuOrder = 0;              // 操作方向
        cfg->autoStartAppName = "None"; // 无指定开机自启APP
        this->WriteConfigToFlash(cfg);
    } else {
        // 解析数据
        char *param[18] = {0};
        ParseParam(cfgInfo, 18, param);
        cfg->ssid0 = param[0];
        cfg->password0 = param[1];
        cfg->ssid1 = param[2];
        cfg->password1 = param[3];
        cfg->ssid2 = param[4];
        cfg->password2 = param[5];
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
    wData += cfg->ssid0 + "\n";
    wData += cfg->password0 + "\n";
    wData += cfg->ssid1 + "\n";
    wData += cfg->password1 + "\n";
    wData += cfg->ssid2 + "\n";
    wData += cfg->password2 + "\n";
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

    g_flashFs.WriteFile(SYS_CONFIG_PATH, wData.c_str());

    // 立即生效相关配置
    // screen.setBackLight(cfg->backlight / 100.0);
    // tft->setRotation(cfg->rotation);
    // mpu.setOrder(cfg->imuOrder);
}

void AppController::MessageProcess(APP_MESSAGE_TYPE type, const char *key, char *value)
{
    switch (type) {

        case APP_MESSAGE_GET_PARAM:
            if (!strcmp(key, "ssid0")) {
                snprintf(value, 32, "%s", m_sysCfg.ssid0.c_str());
            } else if (!strcmp(key, "password0")) {
                snprintf(value, 32, "%s", m_sysCfg.password0.c_str());
            } else if (!strcmp(key, "ssid1")) {
                snprintf(value, 32, "%s", m_sysCfg.ssid1.c_str());
            } else if (!strcmp(key, "password1")) {
                snprintf(value, 32, "%s", m_sysCfg.password1.c_str());
            } else if (!strcmp(key, "ssid2")) {
                snprintf(value, 32, "%s", m_sysCfg.ssid2.c_str());
            } else if (!strcmp(key, "password2")) {
                snprintf(value, 32, "%s", m_sysCfg.password2.c_str());
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
            break;
        case APP_MESSAGE_SET_PARAM:
            if (!strcmp(key, "ssid0")) {
                m_sysCfg.ssid0 = value;
            } else if (!strcmp(key, "password0")) {
                m_sysCfg.password0 = value;
            } else if (!strcmp(key, "ssid1")) {
                m_sysCfg.ssid1 = value;
            } else if (!strcmp(key, "password1")) {
                m_sysCfg.password1 = value;
            } else if (!strcmp(key, "ssid2")) {
                m_sysCfg.ssid2 = value;
            } else if (!strcmp(key, "password2")) {
                m_sysCfg.password2 = value;
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