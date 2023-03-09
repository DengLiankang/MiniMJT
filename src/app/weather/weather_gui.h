#ifndef APP_WEATHER_GUI_H
#define APP_WEATHER_GUI_H

#include <Arduino.h>
#include "lvgl.h"
#include "common.h"

#define ANIEND_WAIT                     \
    do {                                \
        lv_timer_handler();             \
        delay(1);                       \
    } while (lv_anim_count_running())

struct WEATHER_STRUCT {
    int weatherCode; // 天气现象代码
    int temperature;  // 温度
    int humidity;     // 湿度
    int maxTemp;      // 最高气温
    int minTemp;      // 最低气温
    String windDir;
    String cityName; // 城市名
    int windLevel;
    int airQulity;

    short dailyHighTemp[7];
    short dailyLowTemp[7];
};

enum WEATHER_APP_PAGE {
    CLOCK_PAGE,
    CURVE_PAGE,
    OTHER_PAGE,
};

void weatherAppGuiInit(struct WEATHER_STRUCT weatherInfo, struct tm timeInfo);
enum WEATHER_APP_PAGE GetWeatherAppGuiPage(void);
void WeatherAppGuiPageFlip(lv_scr_load_anim_t anim);
void DisplayTime(struct tm timeInfo);
void DisplayWeather(struct WEATHER_STRUCT weatherInfo);
void DisplaySpaceMan(void);
void DisplayCurve(short maxT[], short minT[]);
void WeatherAppGuiRelease(void);

LV_IMG_DECLARE(WeatherAppLogo);

#endif