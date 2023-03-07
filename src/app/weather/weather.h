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
    long long errorNetTimestamp;    // 网络到显示过程中的时间误差
    long long preLocalTimestamp;    // 上一次的本地机器时间戳
    unsigned int coactusUpdateFlag; // 强制更新标志
    int clock_page;
    unsigned int update_type; // 更新类型的标志位

    struct tm m_timeInfo;
    struct WEATHER_STRUCT m_weatherInfo;     // 保存天气状况
    struct WEATHER_APP_CONFIG m_weatherAppCfg; // 保存app配置
public:
    WeatherApp();

    ~WeatherApp();

    int16_t ReadConfigFromFlash(WEATHER_APP_CONFIG *cfg);

    int8_t WriteConfigToFlash(WEATHER_APP_CONFIG *cfg);

    void WeatherAppDataInit(void);

private:
    
    void ValidateConfig(struct WEATHER_APP_CONFIG *cfg, const struct WEATHER_APP_CONFIG *defaultConfig);
};

extern APP_OBJ weather_app;

#endif