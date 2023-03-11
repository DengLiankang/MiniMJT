#include "Arduino.h"
#include "sys/app_controller.h"

const char *AppMessageEventStr[] = {"APP_MESSAGE_WIFI_CONNECT",    // 开启连接
                                    "APP_MESSAGE_WIFI_CONNECTED",  // 连接成功
                                    "APP_MESSAGE_WIFI_AP_START",   // 开启AP事件
                                    "APP_MESSAGE_WIFI_KEEP_ALIVE", // wifi开关的心跳维持
                                    "APP_MESSAGE_WIFI_DISCONNECT", // 连接断开
                                    "APP_MESSAGE_MQTT_DATA",       // MQTT客户端收到消息
                                    "APP_MESSAGE_GET_PARAM",       // 获取参数
                                    "APP_MESSAGE_SET_PARAM",       // 设置参数
                                    "APP_MESSAGE_READ_CFG",        // 向磁盘读取参数
                                    "APP_MESSAGE_WRITE_CFG",       // 向磁盘写入参数
                                    "APP_MESSAGE_NONE"};

// request interface
int AppController::SendRequestEvent(const char *from, const char *to, APP_MESSAGE_TYPE type, void *data, void *extData)
{
    APP_OBJ *fromApp = GetAppByName(from); // 来自谁 有可能为空
    APP_OBJ *toApp = GetAppByName(to);     // 发送给谁 有可能为空

    if ((strcmp(from, CTRL_NAME) && fromApp == NULL) || ((strcmp(to, CTRL_NAME) && toApp == NULL)))
        return -1;

    Serial.printf("[Massage]%s To %s: %s\n", from, to, AppMessageEventStr[type]);
    if (strcmp(to, CTRL_NAME) == 0) {
        m_requestFrom = from;
        RequestProcess(type, data, extData);
    } else if (toApp->MessageHandle != NULL) {
        toApp->MessageHandle(from, to, type, data, extData);
    }

    return 0;
}
