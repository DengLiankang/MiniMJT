#include "common.h"

SdCard g_tfCard;
TFT_eSPI *tft;
TaskHandle_t gTaskLvglHandle;

// lvgl handle的锁
SemaphoreHandle_t lvgl_mutex = xSemaphoreCreateMutex();

void LvglTask(void *pvParameters)
{
    while (!lv_is_initialized()) {
        delay(1);
    }
    while (1) {
        MJT_LVGL_OPERATE_LOCK(lv_timer_handler());
        delay(1);
    }
}

void InitLvglTaskSetup(const char *name)
{
    xTaskCreate(LvglTask, name, 8 * 1024, NULL, TASK_LVGL_PRIORITY, &gTaskLvglHandle);
}

void DeleteLvglTask(void) { vTaskDelete(gTaskLvglHandle); }

boolean DoDelayMillisTime(unsigned long interval, unsigned long *previousMillis)
{
    unsigned long currentMillis = millis();
    if (currentMillis - *previousMillis >= interval) {
        *previousMillis = currentMillis;
        return true;
    }
    return false;
}

int16_t ReadConfigFromCard(const char *appName, uint8_t *cfg)
{
    if (cfg == NULL) {
        Serial.println("[ERROR] cfg is NULL");
        return -1;
    }
    if (appName == MJT_APP_CTRL) {
        return g_tfCard.ReadFile("/.system/system.cfg", cfg);
    }
    String appNameStr(appName);
    appNameStr.toLowerCase();
    String cfgPath = "/" + appNameStr + "/" + appNameStr + ".cfg";
    return g_tfCard.ReadFile(cfgPath.c_str(), cfg);
}

int8_t WriteConfigToCard(const char *appName, const char *cfg)
{
    if (cfg == NULL) {
        Serial.println("[ERROR] cfg is NULL");
        return -1;
    }
    if (appName == MJT_APP_CTRL) {
        return g_tfCard.WriteFile("/.system/system.cfg", cfg);
    }
    String appNameStr(appName);
    appNameStr.toLowerCase();
    String cfgPath = "/" + appNameStr + "/" + appNameStr + ".cfg";
    return g_tfCard.WriteFile(cfgPath.c_str(), cfg);
}

void ToString(SysUtilConfig *cfg, String &result)
{
    if (cfg == NULL) {
        Serial.println("[ERROR] cfg is NULL");
        return;
    }
    result = "";
    result += "ssid0:" + cfg->ssid[0];
    result += "\npassword0:" + cfg->password[0];
    result += "\nssid1:" + cfg->ssid[1];
    result += "\npassword1:" + cfg->password[1];
    result += "\nssid2:" + cfg->ssid[2];
    result += "\npassword2:" + cfg->password[2];
    result += "\nautoStart:" + cfg->autoStartAppName;
    result += "\npowerMode:" + String(cfg->powerMode);
    result += "\nbacklight:" + String(cfg->backlight);
    result += "\nrotation:" + String(cfg->rotation);
    result += "\nimuAutoCali:" + String(cfg->imuAutoCalibration);
    result += "\nimuOrder:" + String(cfg->imuOrder);
    result += "\nimuGyroX:" + String(cfg->imuOffsets.imuGyroOffsetX);
    result += "\nimuGyroY:" + String(cfg->imuOffsets.imuGyroOffsetY);
    result += "\nimuGyroZ:" + String(cfg->imuOffsets.imuGyroOffsetZ);
    result += "\nimuAccelX:" + String(cfg->imuOffsets.imuAccelOffsetX);
    result += "\nimuAccelY:" + String(cfg->imuOffsets.imuAccelOffsetY);
    result += "\nimuAccelZ:" + String(cfg->imuOffsets.imuAccelOffsetZ);
    Serial.println(result);
}

void fromString(const char *cfgStr, SysUtilConfig *cfg)
{
    String tmpStr(cfgStr);
    cfg->ssid[0] = SplitCfgString(tmpStr);
    cfg->password[0] = SplitCfgString(tmpStr);
    cfg->ssid[1] = SplitCfgString(tmpStr);
    cfg->password[1] = SplitCfgString(tmpStr);
    cfg->ssid[2] = SplitCfgString(tmpStr);
    cfg->password[2] = SplitCfgString(tmpStr);
    cfg->autoStartAppName = SplitCfgString(tmpStr);
    cfg->powerMode = SplitCfgString(tmpStr).toInt();
    cfg->backlight = SplitCfgString(tmpStr).toInt();
    cfg->rotation = SplitCfgString(tmpStr).toInt();
    cfg->imuAutoCalibration = SplitCfgString(tmpStr).toInt();
    cfg->imuOrder = SplitCfgString(tmpStr).toInt();
    cfg->imuOffsets.imuGyroOffsetX = SplitCfgString(tmpStr).toInt();
    cfg->imuOffsets.imuGyroOffsetY = SplitCfgString(tmpStr).toInt();
    cfg->imuOffsets.imuGyroOffsetZ = SplitCfgString(tmpStr).toInt();
    cfg->imuOffsets.imuAccelOffsetX = SplitCfgString(tmpStr).toInt();
    cfg->imuOffsets.imuAccelOffsetY = SplitCfgString(tmpStr).toInt();
    cfg->imuOffsets.imuAccelOffsetZ = SplitCfgString(tmpStr).toInt();
    ToString(cfg, tmpStr);
    Serial.println(tmpStr);
}
