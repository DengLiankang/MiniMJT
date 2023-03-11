#ifndef APP_WEATHER_H
#define APP_WEATHER_H

#include "sys/interface.h"
#include "ESP32Time.h"
#include "weather_gui.h"

struct WEATHER_APP_CONFIG {
    String weatherApiAppId;                 // tianqiapid 的 appid
    String weatherApiAppSecret;             // tianqiapid 的 appsecret
    String weatherApiCityAddr;                  // tianqiapid 的地址（填中文）
    unsigned long weatherUpdataInterval; // 天气更新的时间间隔(s)
    unsigned long timeUpdataInterval;    // 日期时钟更新的时间间隔(s)
};

class WeatherApp{
public:
    ESP32Time m_rtcTime; // 用于时间解码
    unsigned long preWeatherMillis; // 上一回更新天气时的毫秒数
    unsigned long preTimeMillis;    // 更新时间计数器
    long long preNetTimestamp;      // 上一次的网络时间戳
    unsigned long errorNetTimestamp;    // 网络到显示过程中的时间误差
    unsigned long preLocalTimestamp;    // 上一次的本地机器时间戳
    uint8_t m_wifiStatus; // wifi标志
    boolean m_weatherUpdateFlag;
    boolean m_timeUpdateFlag;
    boolean m_forceUpdate;
    unsigned long m_lastKeepWifiMillis;

    struct tm m_timeInfo;
    struct WEATHER_STRUCT m_weatherInfo;     // 保存天气状况
public:
    WeatherApp();

    ~WeatherApp();

    void WeatherAppDataInit(void);

private:

    void ValidateConfig(struct WEATHER_APP_CONFIG *cfg, const struct WEATHER_APP_CONFIG *defaultConfig);
};

extern APP_OBJ weather_app;

#endif