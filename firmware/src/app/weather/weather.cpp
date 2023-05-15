#include "weather.h"
#include "ArduinoJson.h"
#include "sys/app_controller.h"
#include <esp32-hal-timer.h>

#define WEATHER_APP_NAME "Weather"
#define WEATHER_DALIY_API "https://www.yiketianqi.com/free/week?unescape=1&appid=%s&appsecret=%s&city=%s"
#define TIME_API "http://api.m.taobao.com/rest/api3.do?api=mtop.common.gettimestamp"

WeatherApp *g_weatherApp = NULL;
// app cfg一定保证静态全局，防止被外部访问出错
static WEATHER_APP_CONFIG g_weatherAppCfg;

static void UpdateTask(void *pvParameters)
{
    g_weatherApp->GetNowWeather();
    g_weatherApp->GetDailyWeather(g_weatherApp->m_weatherInfo.dailyHighTemp, g_weatherApp->m_weatherInfo.dailyLowTemp);
    struct timeval tv = g_weatherApp->GetNetworkTime();
    g_weatherApp->UpdateTimeInfo(&tv);
    g_weatherApp->m_lastUpdateLocalTimeMillis = millis();
    g_weatherApp->m_weatherUpdateFlag = true;
    g_weatherApp->m_timeUpdateFlag = true;
    vTaskDelete(g_weatherApp->m_updateTaskHandle);
}

static void ToString(WEATHER_APP_CONFIG *cfg, String &result)
{
    if (cfg == NULL) {
        Serial.println("[ERROR] cfg is NULL");
        return;
    }
    result = "";
    result += "appID:" + cfg->weatherApiAppId;
    result += "\nappSecret:" + cfg->weatherApiAppSecret;
    result += "\ncityAddr:" + cfg->weatherApiCityAddr;
    result += "\nupdateInterval:" + String(cfg->httpUpdataInterval);
}

static void fromString(const char *cfgStr, WEATHER_APP_CONFIG *cfg)
{
    String tmpStr(cfgStr);
    cfg->weatherApiAppId = SplitCfgString(tmpStr);
    cfg->weatherApiAppSecret = SplitCfgString(tmpStr);
    cfg->weatherApiCityAddr = SplitCfgString(tmpStr);
    cfg->httpUpdataInterval = SplitCfgString(tmpStr).toInt();
}

static int8_t WriteConfig(WEATHER_APP_CONFIG *cfg)
{
    if (cfg == NULL) {
        return -1;
    }
    String cfgStr;
    ToString(cfg, cfgStr);
    return WriteConfigToCard(WEATHER_APP_NAME, cfgStr.c_str());
}

static int16_t ReadConfig(WEATHER_APP_CONFIG *cfg)
{
    if (cfg == NULL) {
        return -1;
    }
    String cfgStr;
    uint8_t tmpStr[100];
    int16_t size = ReadConfigFromCard(WEATHER_APP_NAME, tmpStr);
    if (size > 0) {
        fromString((const char *)tmpStr, cfg);
    }
    return size;
}

static void GetParam(const char *key, char *value)
{
    if (!strcmp(key, "weatherApiAppId")) {
        snprintf((char *)value, 32, "%s", g_weatherAppCfg.weatherApiAppId.c_str());
    } else if (!strcmp(key, "weatherApiAppSecret")) {
        snprintf((char *)value, 32, "%s", g_weatherAppCfg.weatherApiAppSecret.c_str());
    } else if (!strcmp(key, "weatherApiCityAddr")) {
        snprintf((char *)value, 32, "%s", g_weatherAppCfg.weatherApiCityAddr.c_str());
    } else if (!strcmp(key, "httpUpdataInterval")) {
        snprintf((char *)value, 32, "%lu", g_weatherAppCfg.httpUpdataInterval);
    } else {
        snprintf((char *)value, 32, "%s", "NULL");
    }
}

static void SetParam(const char *key, const char *value)
{
    if (!strcmp(key, "weatherApiAppId")) {
        g_weatherAppCfg.weatherApiAppId = value;
    } else if (!strcmp(key, "weatherApiAppSecret")) {
        g_weatherAppCfg.weatherApiAppSecret = value;
    } else if (!strcmp(key, "weatherApiCityAddr")) {
        g_weatherAppCfg.weatherApiCityAddr = value;
    } else if (!strcmp(key, "httpUpdataInterval")) {
        g_weatherAppCfg.httpUpdataInterval = atol(value);
    }
}

static int AirQulityLevel(int q)
{
    int ret = q / 50;
    return ret > 5 ? 5 : ret;
}

WeatherApp::WeatherApp()
{
    struct WEATHER_APP_CONFIG defaultConfig = {
        .weatherApiAppId = "22513773",
        .weatherApiAppSecret = "rq2r6sXd",
        .weatherApiCityAddr = "武冈",
        .httpUpdataInterval = 900000,
    };
    struct WEATHER_STRUCT defaultWeather = {
        .weatherCode = "qing",
        .temperature = 18,
        .humidity = 38,
        .maxTemp = 25,
        .minTemp = 13,
        .windDir = "西北风",
        .windSpeed = "3级",
        .cityName = "武冈",
        .airQulity = 0,

        .dailyHighTemp = {23, 24, 26, 24, 23, 25, 22},
        .dailyLowTemp = {13, 15, 14, 12, 15, 16, 12},
    };
    ValidateConfig(&g_weatherAppCfg, &defaultConfig);
    UpdateTimeInfo();
    memcpy(&m_weatherInfo, &defaultWeather, sizeof(struct WEATHER_STRUCT));
    m_weatherInfo.cityName = g_weatherAppCfg.weatherApiCityAddr;
    m_lastUpdateLocalTimeMillis = millis(); // 上一次的本地机器时间戳
    m_lastKeepWifiMillis = 0;
    m_lastHttpUpdateMillis = 0;
    m_wifiStatus = WIFI_STATUS::WIFI_DISCONNECTED;
    m_wifiRetryCnt = 0;
    m_weatherUpdateFlag = false;
    m_timeUpdateFlag = false;
    m_forceUpdate = true;
    m_updateTaskHandle = NULL;
}
WeatherApp::~WeatherApp() {}

void WeatherApp::GetNowWeather(void)
{
    HTTPClient http;
    http.setTimeout(500);
    String api = "https://www.yiketianqi.com/free/day?unescape=1&appid=" + g_weatherAppCfg.weatherApiAppId +
                 "&appsecret=" + g_weatherAppCfg.weatherApiAppSecret + "&city=" + g_weatherAppCfg.weatherApiCityAddr;
    Serial.printf("API = %s\n", api.c_str());
    http.begin(api);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.println(payload);
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        JsonObject sk = doc.as<JsonObject>();
        g_weatherApp->m_weatherInfo.cityName = sk["city"].as<String>();
        g_weatherApp->m_weatherInfo.weatherCode = sk["wea_img"].as<String>();
        g_weatherApp->m_weatherInfo.temperature = sk["tem"].as<int>();
        String humidity = sk["humidity"].as<String>();
        g_weatherApp->m_weatherInfo.humidity = atoi(humidity.substring(0, humidity.length() - 1).c_str());

        g_weatherApp->m_weatherInfo.maxTemp = sk["tem_day"].as<int>();
        g_weatherApp->m_weatherInfo.minTemp = sk["tem_night"].as<int>();
        g_weatherApp->m_weatherInfo.windDir = sk["win"].as<String>();
        g_weatherApp->m_weatherInfo.windSpeed = sk["win_speed"].as<String>();
        g_weatherApp->m_weatherInfo.airQulity = AirQulityLevel(sk["air"].as<int>());
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

void WeatherApp::GetDailyWeather(short maxT[], short minT[])
{
    HTTPClient http;
    http.setTimeout(500);
    String api = "https://www.yiketianqi.com/free/week?unescape=1&appid=" + g_weatherAppCfg.weatherApiAppId +
                 "&appsecret=" + g_weatherAppCfg.weatherApiAppSecret + "&city=" + g_weatherAppCfg.weatherApiCityAddr;
    Serial.printf("API = %s\n", api.c_str());
    http.begin(api);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.println(payload);
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);
        JsonObject sk = doc.as<JsonObject>();
        for (int i = 0; i < 7; ++i) {
            maxT[i] = sk["data"][i]["tem_day"].as<int>();
            minT[i] = sk["data"][i]["tem_night"].as<int>();
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

struct timeval WeatherApp::GetNetworkTime(void)
{
    HTTPClient http;
    http.setTimeout(500);
    http.begin(TIME_API);
    struct timeval tv;

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
        DynamicJsonDocument doc(256);
        deserializeJson(doc, payload);
        JsonObject timeObj = doc.as<JsonObject>();
        long long timestamp = timeObj["data"]["t"];
        timestamp += TIMEZERO_OFFSIZE;
        tv.tv_sec = (unsigned long)(timestamp / 1000);
        tv.tv_usec = (unsigned long)(timestamp % 1000);
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        // 得不到网络时间戳时
        gettimeofday(&tv, NULL);
    }
    http.end();

    return tv;
}

void WeatherApp::UpdateTimeInfo(struct timeval *tv)
{
    if (tv != NULL) {
        settimeofday(tv, NULL);
    }
    time_t now;
    time(&now);
    m_timeInfo = localtime(&now);
}

void WeatherApp::ValidateConfig(struct WEATHER_APP_CONFIG *cfg, const struct WEATHER_APP_CONFIG *defaultConfig)
{
    if (cfg->weatherApiAppId == "") {
        cfg->weatherApiAppId = defaultConfig->weatherApiAppId;
    }
    if (cfg->weatherApiAppSecret == "") {
        cfg->weatherApiAppSecret = defaultConfig->weatherApiAppSecret;
    }
    if (cfg->weatherApiCityAddr == "") {
        cfg->weatherApiCityAddr = defaultConfig->weatherApiCityAddr;
    }
    if (cfg->httpUpdataInterval == 0) {
        cfg->httpUpdataInterval = defaultConfig->httpUpdataInterval;
    }
}

static int WeatherAppInit(AppController *sys)
{
    ReadConfig(&g_weatherAppCfg);
    g_weatherApp = new WeatherApp();
    WeatherAppGuiInit(g_weatherApp->m_weatherInfo, *g_weatherApp->m_timeInfo);

    return 0;
}

static void WeatherAppMainProcess(AppController *sys, const ImuAction *act_info)
{
    if (g_weatherApp == NULL)
        return;

    if (RETURN == act_info->active) {
        sys->AppExit();
        return;
    }

    if (TURN_RIGHT == act_info->active || TURN_LEFT == act_info->active) {
        uint8_t curPage = GetWeatherAppGuiPage();
        // 提前刷新
        if (curPage == WEATHER_APP_PAGE::CLOCK_PAGE) {
            DisplayCurve(g_weatherApp->m_weatherInfo.dailyHighTemp, g_weatherApp->m_weatherInfo.dailyLowTemp);
            g_weatherApp->m_weatherUpdateFlag = true;
        } else if (curPage == WEATHER_APP_PAGE::CURVE_PAGE) {
            DisplayTime(*g_weatherApp->m_timeInfo);
            DisplayWeather(g_weatherApp->m_weatherInfo);
            DisplaySpaceMan();
            g_weatherApp->m_weatherUpdateFlag = true;
        }
        lv_scr_load_anim_t anim =
            TURN_RIGHT == act_info->active ? LV_SCR_LOAD_ANIM_MOVE_RIGHT : LV_SCR_LOAD_ANIM_MOVE_LEFT;
        lv_anim_del_all();
        WeatherAppGuiPageFlip(anim);
    }

    uint8_t curPage = GetWeatherAppGuiPage();
    if (curPage == WEATHER_APP_PAGE::CLOCK_PAGE) {
        if (g_weatherApp->m_weatherUpdateFlag) {
            DisplayWeather(g_weatherApp->m_weatherInfo);
            g_weatherApp->m_weatherUpdateFlag = false;
        }
        if (g_weatherApp->m_timeUpdateFlag || DoDelayMillisTime(500, &g_weatherApp->m_lastUpdateLocalTimeMillis)) {
            g_weatherApp->UpdateTimeInfo();
            DisplayTime(*g_weatherApp->m_timeInfo);
            g_weatherApp->m_timeUpdateFlag = false;
        }
        DisplaySpaceMan();
    } else if (curPage == WEATHER_APP_PAGE::CURVE_PAGE) {
        if (g_weatherApp->m_weatherUpdateFlag) {
            DisplayCurve(g_weatherApp->m_weatherInfo.dailyHighTemp, g_weatherApp->m_weatherInfo.dailyLowTemp);
            g_weatherApp->m_weatherUpdateFlag = false;
        }
    }

    if (g_weatherApp->m_wifiStatus == WIFI_STATUS::WIFI_CONNECTED) {
        if (g_weatherApp->m_forceUpdate == true ||
            DoDelayMillisTime(g_weatherAppCfg.httpUpdataInterval, &g_weatherApp->m_lastHttpUpdateMillis)) {
            if (g_weatherApp->m_updateTaskHandle == NULL) {
                xTaskCreate(UpdateTask, "HttpUpdateTask", 8 * 1024, NULL, 0, &g_weatherApp->m_updateTaskHandle);
            }
        }
        g_weatherApp->m_forceUpdate = false;
    }

    if (DoDelayMillisTime(45000, &g_weatherApp->m_lastKeepWifiMillis)) {
        if (g_weatherApp->m_wifiStatus == WIFI_STATUS::WIFI_CONNECTED) {
            sys->SendRequestEvent(WEATHER_APP_NAME, MJT_APP_CTRL, APP_MESSAGE_WIFI_KEEP_ALIVE, NULL, NULL);
            g_weatherApp->m_wifiRetryCnt = 0;
        } else if (++g_weatherApp->m_wifiRetryCnt <= 3) {
            Serial.printf("[Weather] try to connect wifi: %d...", g_weatherApp->m_wifiRetryCnt);
            sys->SendRequestEvent(WEATHER_APP_NAME, MJT_APP_CTRL, APP_MESSAGE_WIFI_CONNECT, NULL, NULL);
        }
    }
}

static int WeatherAppExit(void *param)
{
    WeatherAppGuiRelease();
    WriteConfig(&g_weatherAppCfg);
    if (g_weatherApp->m_updateTaskHandle != NULL) {
        vTaskDelete(g_weatherApp->m_updateTaskHandle);
    }
    if (g_weatherApp != NULL) {
        delete (g_weatherApp);
        g_weatherApp = NULL;
    }

    return 0;
}

static void WeatherAppMessageHandle(const char *from, const char *to, APP_MESSAGE_TYPE type, void *data, void *extData)
{
    switch (type) {
        case APP_MESSAGE_WIFI_CONNECTED:
            if (g_weatherApp != NULL) {
                g_weatherApp->m_wifiStatus = WIFI_STATUS::WIFI_CONNECTED;
            }
            break;
        case APP_MESSAGE_WIFI_DISCONNECT:
            if (g_weatherApp != NULL) {
                g_weatherApp->m_wifiStatus = WIFI_STATUS::WIFI_DISCONNECTED;
            }
            break;
        case APP_MESSAGE_GET_PARAM:
            GetParam((const char *)data, (char *)extData);
            break;
        case APP_MESSAGE_SET_PARAM:
            SetParam((const char *)data, (const char *)extData);
            break;
        case APP_MESSAGE_READ_CFG:
            ReadConfig(&g_weatherAppCfg);
            break;
        case APP_MESSAGE_WRITE_CFG:
            WriteConfig(&g_weatherAppCfg);
            break;
        default:
            break;
    }
}

APP_OBJ WEATHER_APP = {WEATHER_APP_NAME,       "", WeatherAppInit, WeatherAppMainProcess, NULL, WeatherAppExit,
                       WeatherAppMessageHandle};