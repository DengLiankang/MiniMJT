#include "driver/network.h"
#include "HardwareSerial.h"
#include "common.h"

IPAddress local_ip(192, 168, 4, 2); // Set your server's fixed IP address here
IPAddress gateway(192, 168, 4, 2);  // Set your network Gateway usually your Router base address
IPAddress subnet(255, 255, 255, 0); // Set your network sub-network mask here
IPAddress dns(192, 168, 4, 1);      // Set your network DNS usually your Router base address

TimerHandle_t xTimer_ap;

Network::Network()
{
    m_preDisWifiConnInfoMillis = 0;
    WiFi.enableSTA(false);
    WiFi.enableAP(false);
}

void Network::SearchWifi(void)
{
    Serial.println("scan start");
    int wifi_num = WiFi.scanNetworks();
    Serial.println("scan done");
    if (0 == wifi_num) {
        Serial.println("no networks found");
    } else {
        Serial.print(wifi_num);
        Serial.println(" networks found");
        for (int cnt = 0; cnt < wifi_num; ++cnt) {
            Serial.print(cnt + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(cnt));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(cnt));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(cnt) == WIFI_AUTH_OPEN) ? " " : "*");
        }
    }
}

boolean Network::ConnectWifi(const char *ssid, const char *password)
{
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("\nWiFi is already connected.\n"));
        return true;
    }

    Serial.printf("\nConnecting: %s@%s\n", ssid, password);

    // 设置为STA模式并连接WIFI
    WiFi.enableSTA(true);
    // 修改主机名
    WiFi.setHostname(HOST_NAME);
    WiFi.begin(ssid, password);
    m_preDisWifiConnInfoMillis = millis();

    return true;
}

boolean Network::end_conn_wifi(void)
{
    if (WL_CONNECTED != WiFi.status()) {
        if (DoDelayMillisTime(10000, &m_preDisWifiConnInfoMillis)) {
            // 这个if为了减少频繁的打印
            Serial.println(F("\nWiFi connect error.\n"));
        }
        return CONN_ERROR;
    }

    if (DoDelayMillisTime(10000, &m_preDisWifiConnInfoMillis)) {
        // 这个if为了减少频繁的打印
        Serial.println(F("\nWiFi connected"));
        Serial.print(F("IP address: "));
        Serial.println(WiFi.localIP());
    }
    return CONN_SUCC;
}

boolean Network::DisconnectWifi(void)
{
    if (WiFi.getMode() & WIFI_MODE_AP) {
        WiFi.enableAP(false);
        Serial.println(F("AP shutdowm"));
    }

    if (!WiFi.disconnect()) {
        return false;
    }
    WiFi.enableSTA(false);
    WiFi.mode(WIFI_MODE_NULL);
    // esp_wifi_set_inactive_time(ESP_IF_ETH, 10); //设置暂时休眠时间
    // esp_wifi_get_ant(wifi_ant_config_t * config);                   //获取暂时休眠时间
    // WiFi.setSleep(WIFI_PS_MIN_MODEM);
    // WiFi.onEvent();
    Serial.println(F("WiFi disconnect"));
    return true;
}

boolean Network::OpenAp(const char *ap_ssid, const char *ap_password)
{
    WiFi.enableAP(true); // 配置为AP模式
    // 修改主机名
    WiFi.setHostname(HOST_NAME);
    // WiFi.begin();
    boolean result = WiFi.softAP(ap_ssid, ap_password); // 开启WIFI热点
    if (result) {
        WiFi.softAPConfig(local_ip, gateway, subnet);
        IPAddress myIP = WiFi.softAPIP();

        // 打印相关信息
        Serial.print(F("\nSoft-AP IP address = "));
        Serial.println(myIP);
        Serial.println(String("MAC address = ") + WiFi.softAPmacAddress().c_str());
    } else {
        // 开启热点失败
        Serial.println(F("WiFiAP Failed"));
        return false;
    }
    // 设置域名
    if (MDNS.begin(HOST_NAME)) {
        Serial.println(F("MDNS responder started"));
    }
    return true;
}
