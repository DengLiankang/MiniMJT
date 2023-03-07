#include "weather.h"
#include "ArduinoJson.h"
#include "sys/app_controller.h"
#include <esp32-hal-timer.h>
#include <map>

#define WEATHER_APP_NAME "Weather"
#define WEATHER_NOW_API "https://www.yiketianqi.com/free/day?appid=%s&appsecret=%s&unescape=1&city=%s"
#define WEATHER_NOW_API_UPDATE "https://yiketianqi.com/api?unescape=1&version=v6&appid=%s&appsecret=%s&city=%s"
#define WEATHER_DALIY_API "https://www.yiketianqi.com/free/week?unescape=1&appid=%s&appsecret=%s&city=%s"
#define TIME_API "http://api.m.taobao.com/rest/api3.do?api=mtop.common.gettimestamp"
#define WEATHER_PAGE_SIZE 2
#define UPDATE_WEATHER 0x01       // 更新天气
#define UPDATE_DALIY_WEATHER 0x02 // 更新每天天气
#define UPDATE_TIME 0x04          // 更新时间

// 天气的持久化配置
#define WEATHER_CONFIG_PATH "/weather_app.cfg"

WeatherApp *g_weatherApp;

enum wea_event_Id { UPDATE_NOW, UPDATE_NTP, UPDATE_DAILY };

std::map<String, int> weatherMap = {{"qing", 0}, {"yin", 1},     {"yu", 2},  {"yun", 3}, {"bingbao", 4},
                                    {"wu", 5},   {"shachen", 6}, {"lei", 7}, {"xue", 8}};

WeatherApp::WeatherApp() {}
WeatherApp::~WeatherApp() {}

int8_t WeatherApp::WriteConfigToFlash(WEATHER_APP_CONFIG *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    w_data = w_data + cfg->weatherApiAppId + "\n";
    w_data = w_data + cfg->weatherApiAppSecret + "\n";
    w_data = w_data + cfg->weatherApiCityAddr + "\n";
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->weatherUpdataInterval);
    w_data += tmp;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->timeUpdataInterval);
    w_data += tmp;
    return g_flashFs.WriteFile(WEATHER_CONFIG_PATH, w_data.c_str());
}

int16_t WeatherApp::ReadConfigFromFlash(WEATHER_APP_CONFIG *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    int16_t size = g_flashFs.ReadFile(WEATHER_CONFIG_PATH, (uint8_t *)info);
    if (size >= 0) {
        info[size] = 0;
        // 解析数据
        char *param[5] = {0};
        ParseParam(info, 5, param);
        cfg->weatherApiAppId = param[0];
        cfg->weatherApiAppSecret = param[1];
        cfg->weatherApiCityAddr = param[2];
        cfg->weatherUpdataInterval = atol(param[3]);
        cfg->timeUpdataInterval = atol(param[4]);
    } else {
        return size;
    }
    return 0;
}

static int WindLevel(String str)
{
    int ret = 0;
    for (char ch : str) {
        if (ch >= '0' && ch <= '9') {
            ret = ret * 10 + (ch - '0');
        }
    }
    return ret;
}

static int AirQulityLevel(int q)
{
    int ret = q / 50;
    return ret > 5 ? 5 : ret;
}

static void get_weather(void)
{
    if (WL_CONNECTED != WiFi.status())
        return;

    HTTPClient http;
    http.setTimeout(1000);
    char api[128] = {0};
    // snprintf(api, 128, WEATHER_NOW_API, g_weatherAppCfg.weatherApiAppId, g_weatherAppCfg.weatherApiAppSecret, g_weatherAppCfg.weatherApiCityAddr);
    snprintf(api, 128, WEATHER_NOW_API_UPDATE, g_weatherApp->m_weatherAppCfg.weatherApiAppId.c_str(), g_weatherApp->m_weatherAppCfg.weatherApiAppSecret.c_str(),
             g_weatherApp->m_weatherAppCfg.weatherApiCityAddr.c_str());
    Serial.print("API = ");
    Serial.println(api);
    http.begin(api);

    int httpCode = http.GET();
    if (httpCode > 0) {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            Serial.println(payload);
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            JsonObject sk = doc.as<JsonObject>();
            g_weatherApp->m_weatherInfo.cityName = sk["city"].as<String>();
            g_weatherApp->m_weatherInfo.weatherCode = weatherMap[sk["wea_img"].as<String>()];
            g_weatherApp->m_weatherInfo.temperature = sk["tem"].as<int>();

            // 获取湿度
            g_weatherApp->m_weatherInfo.humidity = 50;
            char humidity[8] = {0};
            strncpy(humidity, sk["humidity"].as<String>().c_str(), 8);
            humidity[strlen(humidity) - 1] = 0; // 去除尾部的 % 号
            g_weatherApp->m_weatherInfo.humidity = atoi(humidity);

            g_weatherApp->m_weatherInfo.maxTemp = sk["tem1"].as<int>();
            g_weatherApp->m_weatherInfo.minTemp = sk["tem2"].as<int>();
            g_weatherApp->m_weatherInfo.windDir = sk["win"].as<String>();
            g_weatherApp->m_weatherInfo.windLevel = WindLevel(sk["win_speed"].as<String>());
            g_weatherApp->m_weatherInfo.airQulity = AirQulityLevel(sk["air"].as<int>());
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

static long long get_timestamp()
{
    // 使用本地的机器时钟
    g_weatherApp->preNetTimestamp = g_weatherApp->preNetTimestamp + (millis() - g_weatherApp->preLocalTimestamp);
    g_weatherApp->preLocalTimestamp = millis();
    return g_weatherApp->preNetTimestamp;
}

static long long get_timestamp(String url)
{
    if (WL_CONNECTED != WiFi.status())
        return 0;

    String time = "";
    HTTPClient http;
    http.setTimeout(1000);
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);
            int time_index = (payload.indexOf("data")) + 12;
            time = payload.substring(time_index, payload.length() - 3);
            // 以网络时间戳为准
            g_weatherApp->preNetTimestamp = atoll(time.c_str()) + g_weatherApp->errorNetTimestamp + TIMEZERO_OFFSIZE;
            g_weatherApp->preLocalTimestamp = millis();
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        // 得不到网络时间戳时
        g_weatherApp->preNetTimestamp = g_weatherApp->preNetTimestamp + (millis() - g_weatherApp->preLocalTimestamp);
        g_weatherApp->preLocalTimestamp = millis();
    }
    http.end();

    return g_weatherApp->preNetTimestamp;
}

static void get_daliyWeather(short maxT[], short minT[])
{
    if (WL_CONNECTED != WiFi.status())
        return;

    HTTPClient http;
    http.setTimeout(1000);
    char api[128] = {0};
    snprintf(api, 128, WEATHER_DALIY_API, g_weatherApp->m_weatherAppCfg.weatherApiAppId.c_str(), g_weatherApp->m_weatherAppCfg.weatherApiAppSecret.c_str(),
             g_weatherApp->m_weatherAppCfg.weatherApiCityAddr.c_str());
    Serial.print("API = ");
    Serial.println(api);
    http.begin(api);

    int httpCode = http.GET();
    if (httpCode > 0) {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            Serial.println(payload);
            DynamicJsonDocument doc(2048);
            deserializeJson(doc, payload);
            JsonObject sk = doc.as<JsonObject>();
            for (int gDW_i = 0; gDW_i < 7; ++gDW_i) {
                maxT[gDW_i] = sk["data"][gDW_i]["tem_day"].as<int>();
                minT[gDW_i] = sk["data"][gDW_i]["tem_night"].as<int>();
            }
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
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
    if (cfg->weatherUpdataInterval == 0) {
        cfg->weatherUpdataInterval = defaultConfig->weatherUpdataInterval;
    }
    if (cfg->timeUpdataInterval == 0) {
        cfg->timeUpdataInterval = defaultConfig->timeUpdataInterval;
    }
}

static void UpdateTime_RTC(long long timestamp)
{
    struct tm tm;
    g_weatherApp->m_rtcTime.setTime(timestamp / 1000);
    tm = g_weatherApp->m_rtcTime.getTimeStruct();
    DisplayTime(tm);
}

void WeatherApp::WeatherAppDataInit(void)
{
    struct WEATHER_APP_CONFIG defaultConfig = {
        .weatherApiAppId = "22513773",
        .weatherApiAppSecret = "rq2r6sXd",
        .weatherApiCityAddr = "武冈",
        .weatherUpdataInterval = 900000,
        .timeUpdataInterval = 900000,
    };
    struct WEATHER_STRUCT defaultWeather = {
        .weatherCode = 0,
        .temperature = 18,
        .humidity = 38,
        .maxTemp = 25,
        .minTemp = 13,
        .windDir = "西北风",
        .cityName = "武冈",
        .windLevel = 3,
        .airQulity = 0,

        .dailyHighTemp = {23, 24, 26, 24, 23, 25, 22},
        .dailyLowTemp = {13, 15, 14, 12, 15, 16, 12},
    };
    ValidateConfig(&m_weatherAppCfg, &defaultConfig);
    m_timeInfo = m_rtcTime.getTimeStruct();
    memcpy(&m_weatherInfo, &defaultWeather, sizeof(struct WEATHER_STRUCT));
    m_weatherInfo.cityName = m_weatherAppCfg.weatherApiCityAddr;
    preNetTimestamp = 1577808000000; // 上一次的网络时间戳 初始化为2020-01-01 00:00:00
    errorNetTimestamp = 2;
    preLocalTimestamp = millis(); // 上一次的本地机器时间戳
    clock_page = 0;
    preWeatherMillis = 0;
    preTimeMillis = 0;
    coactusUpdateFlag = 0x01; // 强制更新
    update_type = 0x00; // 表示什么也不需要更新

}

static int WeatherAppInit(AppController *sys)
{
    g_weatherApp = new WeatherApp();
    g_weatherApp->ReadConfigFromFlash(&g_weatherApp->m_weatherAppCfg);
    g_weatherApp->WeatherAppDataInit();
    weatherAppGuiInit(g_weatherApp->m_weatherInfo, g_weatherApp->m_timeInfo);

    return 0;
}

static void WeatherAppMainProcess(AppController *sys, const ImuAction *act_info)
{
    if (RETURN == act_info->active) {
        sys->AppExit();
        return;
    }

    if (TURN_RIGHT == act_info->active || TURN_LEFT == act_info->active) {
        uint8_t curPage = GetWeatherAppGuiPage();
        // 提前刷新
        if (curPage == WEATHER_APP_PAGE::CLOCK_PAGE) {
            DisplayCurve(g_weatherApp->m_weatherInfo.dailyHighTemp, g_weatherApp->m_weatherInfo.dailyLowTemp);
        } else if (curPage == WEATHER_APP_PAGE::CURVE_PAGE) {
            DisplayTime(g_weatherApp->m_timeInfo);
            DisplayWeather(g_weatherApp->m_weatherInfo);
            DisplaySpaceMan();
        }
        lv_scr_load_anim_t anim = TURN_RIGHT == act_info->active ? LV_SCR_LOAD_ANIM_MOVE_RIGHT : LV_SCR_LOAD_ANIM_MOVE_LEFT;
        WeatherAppGuiPageFlip(anim);
    }

    uint8_t curPage = GetWeatherAppGuiPage();
    if (curPage == WEATHER_APP_PAGE::CLOCK_PAGE) {
        DisplayWeather(g_weatherApp->m_weatherInfo);
        if (0x01 == g_weatherApp->coactusUpdateFlag ||
            doDelayMillisTime(g_weatherApp->m_weatherAppCfg.weatherUpdataInterval, &g_weatherApp->preWeatherMillis, false)) {
            sys->send_to(WEATHER_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_CONN, (void *)UPDATE_NOW, NULL);
            sys->send_to(WEATHER_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_CONN, (void *)UPDATE_DAILY, NULL);
        }

        if (0x01 == g_weatherApp->coactusUpdateFlag ||
            doDelayMillisTime(g_weatherApp->m_weatherAppCfg.timeUpdataInterval, &g_weatherApp->preTimeMillis, false)) {
            // 尝试同步网络上的时钟
            sys->send_to(WEATHER_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_CONN, (void *)UPDATE_NTP, NULL);
        } else if (millis() - g_weatherApp->preLocalTimestamp > 400) {
            UpdateTime_RTC(get_timestamp());
        }
        g_weatherApp->coactusUpdateFlag = 0x00; // 取消强制更新标志
        DisplaySpaceMan();
    } else if (curPage == WEATHER_APP_PAGE::CURVE_PAGE) {
        DisplayCurve(g_weatherApp->m_weatherInfo.dailyHighTemp, g_weatherApp->m_weatherInfo.dailyLowTemp);
    }
}

static int WeatherAppExit(void *param)
{
    WeatherAppGuiRelease();
    g_weatherApp->WriteConfigToFlash(&g_weatherApp->m_weatherAppCfg);

    delete(g_weatherApp);
    return 0;
}

static void WeatherAppMessageHandle(const char *from, const char *to, APP_MESSAGE_TYPE type, void *message,
                                   void *ext_info)
{
    switch (type) {
        case APP_MESSAGE_WIFI_CONN: {
            Serial.println(F("----->weather_event_notification"));
            int event_id = (int)message;
            switch (event_id) {
                case UPDATE_NOW: {
                    Serial.print(F("weather update.\n"));
                    g_weatherApp->update_type |= UPDATE_WEATHER;

                    get_weather();
                    if (g_weatherApp->clock_page == 0) {
                        DisplayWeather(g_weatherApp->m_weatherInfo);
                    }
                }; break;
                case UPDATE_NTP: {
                    Serial.print(F("ntp update.\n"));
                    g_weatherApp->update_type |= UPDATE_TIME;

                    long long timestamp = get_timestamp(TIME_API); // nowapi时间API
                    if (g_weatherApp->clock_page == 0) {
                        UpdateTime_RTC(timestamp);
                    }
                }; break;
                case UPDATE_DAILY: {
                    Serial.print(F("daliy update.\n"));
                    g_weatherApp->update_type |= UPDATE_DALIY_WEATHER;

                    get_daliyWeather(g_weatherApp->m_weatherInfo.dailyHighTemp, g_weatherApp->m_weatherInfo.dailyLowTemp);
                    if (g_weatherApp->clock_page == 1) {
                        DisplayCurve(g_weatherApp->m_weatherInfo.dailyHighTemp, g_weatherApp->m_weatherInfo.dailyLowTemp);
                    }
                }; break;
                default:
                    break;
            }
        } break;
        case APP_MESSAGE_GET_PARAM: {
            char *param_key = (char *)message;
            if (!strcmp(param_key, "weatherApiAppId")) {
                snprintf((char *)ext_info, 32, "%s", g_weatherApp->m_weatherAppCfg.weatherApiAppId.c_str());
            } else if (!strcmp(param_key, "weatherApiAppSecret")) {
                snprintf((char *)ext_info, 32, "%s", g_weatherApp->m_weatherAppCfg.weatherApiAppSecret.c_str());
            } else if (!strcmp(param_key, "weatherApiCityAddr")) {
                snprintf((char *)ext_info, 32, "%s", g_weatherApp->m_weatherAppCfg.weatherApiCityAddr.c_str());
            } else if (!strcmp(param_key, "weatherUpdataInterval")) {
                snprintf((char *)ext_info, 32, "%lu", g_weatherApp->m_weatherAppCfg.weatherUpdataInterval);
            } else if (!strcmp(param_key, "timeUpdataInterval")) {
                snprintf((char *)ext_info, 32, "%lu", g_weatherApp->m_weatherAppCfg.timeUpdataInterval);
            } else {
                snprintf((char *)ext_info, 32, "%s", "NULL");
            }
        } break;
        case APP_MESSAGE_SET_PARAM: {
            char *param_key = (char *)message;
            char *param_val = (char *)ext_info;
            if (!strcmp(param_key, "weatherApiAppId")) {
                g_weatherApp->m_weatherAppCfg.weatherApiAppId = param_val;
            } else if (!strcmp(param_key, "weatherApiAppSecret")) {
                g_weatherApp->m_weatherAppCfg.weatherApiAppSecret = param_val;
            } else if (!strcmp(param_key, "weatherApiCityAddr")) {
                g_weatherApp->m_weatherAppCfg.weatherApiCityAddr = param_val;
            } else if (!strcmp(param_key, "weatherUpdataInterval")) {
                g_weatherApp->m_weatherAppCfg.weatherUpdataInterval = atol(param_val);
            } else if (!strcmp(param_key, "timeUpdataInterval")) {
                g_weatherApp->m_weatherAppCfg.timeUpdataInterval = atol(param_val);
            }
        } break;
        case APP_MESSAGE_READ_CFG: {
            g_weatherApp->ReadConfigFromFlash(&g_weatherApp->m_weatherAppCfg);
        } break;
        case APP_MESSAGE_WRITE_CFG: {
            g_weatherApp->WriteConfigToFlash(&g_weatherApp->m_weatherAppCfg);
        } break;
        default:
            break;
    }
}

APP_OBJ weather_app = {WEATHER_APP_NAME, &WeatherAppLogo, "",
                       WeatherAppInit, WeatherAppMainProcess, NULL,
                       WeatherAppExit, WeatherAppMessageHandle};