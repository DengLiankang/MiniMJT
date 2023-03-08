#ifndef NETWORK_H
#define NETWORK_H

// 时区偏移(小时) 8*60*60
#define TIMEZERO_OFFSIZE (28800000)

#define CONN_SUCC 0
#define CONN_ERROR 1
#define CONN_ERR_TIMEOUT 15 // 连接WiFi的超时时间（s）

// wifi是否连接标志
#define AP_DISABLE 0
#define AP_ENABLE 1

#define AP_SSID "MiniMJT"
#define HOST_NAME "MiniMJT"

// Set your server's logical name here e.g. if 'myserver' then address is http://myserver.local/
#define SERVER_NAME "fileserver"

#ifdef ESP8266
#include <ESP8266WebServer.h> // Built-in
#include <ESP8266WiFi.h>      // Built-in
#include <ESP8266WiFiMulti.h> // Built-in
#include <ESP8266mDNS.h>
#else
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WebServer.h> // https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include <WiFi.h>      // Built-in
#include <WiFiMulti.h> // 当我们需要使用ESP8266开发板存储多个WiFi网络连接信息时，可以使用ESP8266WiFiMulti库来实现。
#endif

// #ifdef __cplusplus
// extern "C"
// {
// #endif

// #include "WiFiGeneric.h"

// #ifdef __cplusplus
// } /* extern "C" */
// #endif

extern IPAddress local_ip; // Set your server's fixed IP address here
extern IPAddress gateway;  // Set your network Gateway usually your Router base address
extern IPAddress subnet;   // Set your network sub-network mask here
extern IPAddress dns;      // Set your network DNS usually your Router base address

void restCallback(TimerHandle_t xTimer);

class Network
{
private:
    unsigned long m_preDisWifiConnInfoMillis; // 保存上一回显示连接状态的时间戳

public:
    Network();
    void SearchWifi(void);
    boolean ConnectWifi(const char *ssid, const char *password);
    boolean end_conn_wifi(void);
    boolean DisconnectWifi(void);
    boolean OpenAp(const char *ap_ssid = AP_SSID, const char *ap_password = NULL);
};

#endif
