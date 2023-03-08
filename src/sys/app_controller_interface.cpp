#include "Arduino.h"
#include "sys/app_controller.h"

const char *AppMessageEventStr[] = {"APP_MESSAGE_WIFI_CONN",    "APP_MESSAGE_WIFI_AP",     "APP_MESSAGE_WIFI_ALIVE",
                                     "APP_MESSAGE_WIFI_DISCONN", "APP_MESSAGE_UPDATE_TIME", "APP_MESSAGE_MQTT_DATA",
                                     "APP_MESSAGE_GET_PARAM",    "APP_MESSAGE_SET_PARAM",   "APP_MESSAGE_READ_CFG",
                                     "APP_MESSAGE_WRITE_CFG",    "APP_MESSAGE_NONE"};

// request interface
int AppController::SendRequestEvent(const char *from, const char *to, APP_MESSAGE_TYPE type, void *data, void *extData)
{
    APP_OBJ *fromApp = GetAppByName(from); // 来自谁 有可能为空
    APP_OBJ *toApp = GetAppByName(to);     // 发送给谁 有可能为空

    if (fromApp == NULL)
        return -1;

    if (strcmp(from, CTRL_NAME) && toApp == NULL)
        return -1;

    // broadcast
    if (strcmp(from, CTRL_NAME) == 0 && toApp == NULL) {
        Serial.printf("[Massage][Broadcast]%s\n", AppMessageEventStr[type]);
        for (APP_OBJ *app : m_appList) {
            if (app->MessageHandle != NULL) {
                app->MessageHandle(from, app->appName, type, data, extData);
            }
        }
    } else {
        Serial.printf("[Massage]%s To %s: %s\n", fromApp->appName, toApp->appName, AppMessageEventStr[type]);
        if (strcmp(to, CTRL_NAME) == 0) {
            RequestProcess(type, data, extData);
        } else if (toApp->MessageHandle != NULL) {
            toApp->MessageHandle(from, to, type, data, extData);
        }
    }

    return 0;
}


