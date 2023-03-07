#include "weather_gui.h"

LV_FONT_DECLARE(lv_font_ibmplex_115);
LV_FONT_DECLARE(lv_font_ibmplex_64);
LV_FONT_DECLARE(ch_yahei_font_20);
LV_IMG_DECLARE(weather_0);
LV_IMG_DECLARE(weather_9);
LV_IMG_DECLARE(weather_14);
LV_IMG_DECLARE(weather_5);
LV_IMG_DECLARE(weather_25);
LV_IMG_DECLARE(weather_30);
LV_IMG_DECLARE(weather_26);
LV_IMG_DECLARE(weather_11);
LV_IMG_DECLARE(weather_23);
LV_IMG_DECLARE(man_0);
LV_IMG_DECLARE(man_1);
LV_IMG_DECLARE(man_2);
LV_IMG_DECLARE(man_3);
LV_IMG_DECLARE(man_4);
LV_IMG_DECLARE(man_5);
LV_IMG_DECLARE(man_6);
LV_IMG_DECLARE(man_7);
LV_IMG_DECLARE(man_8);
LV_IMG_DECLARE(man_9);
LV_IMG_DECLARE(tempIcon);
LV_IMG_DECLARE(humiIcon);

static lv_obj_t *lv_weatherAppScr1 = NULL;
static lv_obj_t *lv_weatherAppScr2 = NULL;
static lv_obj_t *lv_weatherChart = NULL;
static lv_obj_t *lv_weatherChartLabel = NULL;

static lv_obj_t *lv_weatherIconImg = NULL;
static lv_obj_t *lv_cityLabel = NULL;
static lv_obj_t *lv_airQualityBtn = NULL;
static lv_obj_t *lv_airQualityBtnLabel = NULL;
static lv_obj_t *lv_weatherTextLabel = NULL;
static lv_obj_t *lv_clockLabel = NULL, *lv_secondLabel = NULL;
static lv_obj_t *lv_dateLabel = NULL;
static lv_obj_t *lv_tempImg = NULL, *lv_tempBar = NULL, *lv_tempLabel = NULL;
static lv_obj_t *lv_humiImg = NULL, *lv_humiBar = NULL, *lv_humiLabel = NULL;
static lv_obj_t *lv_spaceManImg = NULL;

static lv_chart_series_t *lv_tempHighSeries, *lv_tempLowSeries;

// 天气图标路径的映射关系
const void *WeatherIconMap[] = {&weather_0,  &weather_9,  &weather_14, &weather_5, &weather_25,
                                &weather_30, &weather_26, &weather_11, &weather_23};
// 太空人图标路径的映射关系
const void *SpaceManImgMap[] = {&man_0, &man_1, &man_2, &man_3, &man_4, &man_5, &man_6, &man_7, &man_8, &man_9};
static uint8_t g_spaceIndex = 0;
static const char *WeekDayCh[] = {"日", "一", "二", "三", "四", "五", "六"};
static const char *AirQualityCh[] = {"优", "良", "轻度", "中度", "重度", "严重"};

void weatherAppGuiInit(struct WEATHER_STRUCT weatherInfo, struct tm timeInfo)
{
    static lv_style_t lv_chFontStyle;
    static lv_style_t lv_secondNumStyle;
    static lv_style_t lv_clockNumStyle;
    static lv_style_t lv_airQualityBtnStyle;
    static lv_style_t lv_humiBarStyle;

    lv_style_init(&lv_chFontStyle);
    lv_style_set_text_opa(&lv_chFontStyle, LV_OPA_COVER);
    lv_style_set_text_color(&lv_chFontStyle, lv_color_hex(0xffffff));
    lv_style_set_text_font(&lv_chFontStyle, &ch_yahei_font_20);

    lv_style_init(&lv_secondNumStyle);
    lv_style_set_text_opa(&lv_secondNumStyle, LV_OPA_COVER);
    lv_style_set_text_color(&lv_secondNumStyle, lv_color_hex(0xffffff));
    lv_style_set_text_font(&lv_secondNumStyle, &lv_font_ibmplex_64);

    lv_style_init(&lv_clockNumStyle);
    lv_style_set_text_opa(&lv_clockNumStyle, LV_OPA_COVER);
    lv_style_set_text_color(&lv_clockNumStyle, lv_color_hex(0xffffff));
    lv_style_set_text_font(&lv_clockNumStyle, &lv_font_ibmplex_115);

    lv_style_init(&lv_airQualityBtnStyle);
    lv_style_set_border_width(&lv_airQualityBtnStyle, 0);

    lv_style_init(&lv_humiBarStyle);
    lv_style_set_bg_color(&lv_humiBarStyle, lv_color_hex(0x000000));
    lv_style_set_border_width(&lv_humiBarStyle, 1);
    lv_style_set_border_color(&lv_humiBarStyle, lv_color_hex(0xFFFFFF));
    lv_style_set_pad_all(&lv_humiBarStyle, 2);

    // 天气时钟页
    lv_weatherAppScr1 = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(lv_weatherAppScr1, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_size(lv_weatherAppScr1, 240, 240);
    lv_obj_align(lv_weatherAppScr1, LV_ALIGN_CENTER, 0, 0);

    // 天气图标
    lv_weatherIconImg = lv_img_create(lv_weatherAppScr1);
    lv_img_set_src(lv_weatherIconImg, WeatherIconMap[weatherInfo.weatherCode]);

    // 城市
    lv_cityLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_cityLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_label_set_recolor(lv_cityLabel, true);
    lv_label_set_text(lv_cityLabel, weatherInfo.cityName.c_str());

    // 空气质量
    lv_airQualityBtn = lv_btn_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_airQualityBtn, &lv_airQualityBtnStyle, LV_STATE_DEFAULT);
    lv_obj_set_pos(lv_airQualityBtn, 75, 15);
    lv_obj_set_size(lv_airQualityBtn, 50, 25);
    lv_obj_set_style_bg_color(lv_airQualityBtn, lv_palette_main(LV_PALETTE_ORANGE), LV_STATE_DEFAULT);

    // 空气质量文字
    lv_airQualityBtnLabel = lv_label_create(lv_airQualityBtn);
    lv_obj_add_style(lv_airQualityBtnLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_obj_align(lv_airQualityBtnLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(lv_airQualityBtnLabel, AirQualityCh[weatherInfo.airQulity]);

    // 天气文字
    lv_weatherTextLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_weatherTextLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_obj_set_size(lv_weatherTextLabel, 120, 30);
    lv_label_set_long_mode(lv_weatherTextLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text_fmt(lv_weatherTextLabel, "最低气温%d°C, 最高气温%d°C, %s%d级.   ", weatherInfo.minTemp, weatherInfo.maxTemp, weatherInfo.windDir.c_str(), weatherInfo.windLevel);

    // 时间 时分
    lv_clockLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_clockLabel, &lv_clockNumStyle, LV_STATE_DEFAULT);
    lv_label_set_recolor(lv_clockLabel, true);
    lv_label_set_text_fmt(lv_clockLabel, "%02d#ffa500 %02d#", timeInfo.tm_hour, timeInfo.tm_min);

    // 时间 秒
    lv_secondLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_secondLabel, &lv_secondNumStyle, LV_STATE_DEFAULT);
    lv_label_set_recolor(lv_secondLabel, true);
    lv_label_set_text_fmt(lv_secondLabel, "%02d", timeInfo.tm_sec);

    // 日历
    lv_dateLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_dateLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(lv_dateLabel, "%2d月%2d日   周%s", 11, 23, WeekDayCh[timeInfo.tm_wday]);

    // 温度图标
    lv_tempImg = lv_img_create(lv_weatherAppScr1);
    lv_img_set_src(lv_tempImg, &tempIcon);
    // 温度条
    lv_tempBar = lv_bar_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_tempBar, &lv_humiBarStyle, LV_STATE_DEFAULT);
    lv_bar_set_range(lv_tempBar, -50, 50); // 设置进度条表示的温度为-50~50
    lv_obj_set_size(lv_tempBar, 60, 12);
    lv_obj_set_style_bg_color(lv_tempBar, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
    lv_bar_set_value(lv_tempBar, weatherInfo.temperature, LV_ANIM_ON);
    // 温度文字
    lv_tempLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_tempLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(lv_tempLabel, "%2d°C", weatherInfo.temperature);

    // 湿度图标
    lv_humiImg = lv_img_create(lv_weatherAppScr1);
    lv_img_set_src(lv_humiImg, &humiIcon);
    // 湿度条
    lv_humiBar = lv_bar_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_humiBar, &lv_humiBarStyle, LV_STATE_DEFAULT);
    lv_bar_set_range(lv_humiBar, 0, 100);
    lv_obj_set_size(lv_humiBar, 60, 12);
    lv_obj_set_style_bg_color(lv_humiBar, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);
    lv_bar_set_value(lv_humiBar, weatherInfo.humidity, LV_ANIM_ON);
    // 湿度文字
    lv_humiLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_humiLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(lv_humiLabel, "%2d%", weatherInfo.humidity);

    // 太空人图标
    lv_spaceManImg = lv_img_create(lv_weatherAppScr1);
    lv_img_set_src(lv_spaceManImg, SpaceManImgMap[g_spaceIndex]);

    // 图形对齐
    lv_obj_align(lv_weatherIconImg, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_align(lv_cityLabel, LV_ALIGN_TOP_LEFT, 20, 15);
    lv_obj_align(lv_weatherTextLabel, LV_ALIGN_TOP_LEFT, 10, 50);
    lv_obj_align(lv_tempImg, LV_ALIGN_LEFT_MID, 10, 70);
    lv_obj_align(lv_tempBar, LV_ALIGN_LEFT_MID, 35, 70);
    lv_obj_align(lv_tempLabel, LV_ALIGN_LEFT_MID, 105, 70);
    lv_obj_align(lv_humiImg, LV_ALIGN_LEFT_MID, 10, 100);
    lv_obj_align(lv_humiBar, LV_ALIGN_LEFT_MID, 35, 100);
    lv_obj_align(lv_humiLabel, LV_ALIGN_LEFT_MID, 105, 100);
    lv_obj_align(lv_spaceManImg, LV_ALIGN_BOTTOM_RIGHT, -5, -5);

    lv_obj_align(lv_clockLabel, LV_ALIGN_LEFT_MID, 0, 10);
    lv_obj_align(lv_secondLabel, LV_ALIGN_LEFT_MID, 165, 9);
    lv_obj_align(lv_dateLabel, LV_ALIGN_LEFT_MID, 10, 35);

    // 天气表格页
    lv_weatherAppScr2 = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(lv_weatherAppScr2, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_size(lv_weatherAppScr2, 240, 240);
    lv_obj_align(lv_weatherAppScr2, LV_ALIGN_CENTER, 0, 0);

    // 天气表格描述
    lv_weatherChartLabel = lv_label_create(lv_weatherAppScr2);
    lv_obj_add_style(lv_weatherChartLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_label_set_recolor(lv_weatherChartLabel, true);
    lv_label_set_text(lv_weatherChartLabel, "近7日天气");

    // 天气表格
    lv_weatherChart = lv_chart_create(lv_weatherAppScr2);
    lv_obj_set_size(lv_weatherChart, 220, 180);
    lv_chart_set_range(lv_weatherChart, LV_CHART_AXIS_PRIMARY_Y, -50, 50); // 设置进度条表示的温度为-50~50
    lv_chart_set_point_count(lv_weatherChart, 7);
    lv_chart_set_div_line_count(lv_weatherChart, 5, 7);
    lv_chart_set_type(lv_weatherChart, LV_CHART_TYPE_LINE); /*Show lines and points too*/

    lv_tempHighSeries =
        lv_chart_add_series(lv_weatherChart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
    lv_tempLowSeries =
        lv_chart_add_series(lv_weatherChart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);

    lv_obj_align(lv_weatherChartLabel, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_align(lv_weatherChart, LV_ALIGN_CENTER, 0, 10);

    // default weather scr
    lv_scr_load_anim(lv_weatherAppScr1, LV_SCR_LOAD_ANIM_FADE_IN, 500, 500, false);
}

void WeatherAppGuiRelease(void)
{
    if (lv_weatherAppScr1 != NULL) {
        lv_obj_clean(lv_weatherAppScr1);
        lv_weatherAppScr1 = NULL;
        lv_weatherIconImg = NULL;
        lv_cityLabel = NULL;
        lv_airQualityBtn = NULL;
        lv_airQualityBtnLabel = NULL;
        lv_weatherTextLabel = NULL;
        lv_clockLabel = NULL;
        lv_secondLabel = NULL;
        lv_dateLabel = NULL;
        lv_tempImg = NULL;
        lv_tempBar = NULL;
        lv_tempLabel = NULL;
        lv_humiImg = NULL;
        lv_humiBar = NULL;
        lv_humiLabel = NULL;
        lv_spaceManImg = NULL;
    }

    if (lv_weatherAppScr2 != NULL) {
        lv_obj_clean(lv_weatherAppScr2);
        lv_weatherAppScr2 = NULL;
        lv_weatherChart = NULL;
        lv_weatherChartLabel = NULL;
        lv_tempHighSeries = NULL;
        lv_tempLowSeries = NULL;
    }
}

void DisplaySpaceMan(void)
{
    static unsigned long lastUpdateTime = 0;
    unsigned long curTime = millis();
    if (curTime - lastUpdateTime >= 30000) {
        if (NULL != lv_spaceManImg) {
            lv_img_set_src(lv_spaceManImg, SpaceManImgMap[++g_spaceIndex]);
            g_spaceIndex = g_spaceIndex == 10 ? 0 : g_spaceIndex;
        }
        lastUpdateTime = curTime;
    }
}

void DisplayTime(struct tm timeInfo)
{
    lv_label_set_text_fmt(lv_clockLabel, "%02d#ffa500 %02d#", timeInfo.tm_hour, timeInfo.tm_min);
    lv_label_set_text_fmt(lv_secondLabel, "%02d", timeInfo.tm_sec);
    lv_label_set_text_fmt(lv_dateLabel, "%2d月%2d日   周%s", timeInfo.tm_mon, timeInfo.tm_mday, WeekDayCh[timeInfo.tm_wday]);
}

void WeatherAppGuiPageFlip(lv_scr_load_anim_t anim)
{
    if (lv_weatherAppScr1 == NULL || lv_weatherAppScr2 == NULL)
        return;
    
    if (lv_scr_act() == lv_weatherAppScr1) {
        lv_scr_load_anim(lv_weatherAppScr2, anim, 500, 500, false);
    } else {
        lv_scr_load_anim(lv_weatherAppScr1, anim, 500, 500, false);
    }
    ANIEND_WAIT;
}

enum WEATHER_APP_PAGE GetWeatherAppGuiPage(void)
{
    if (lv_weatherAppScr1 == NULL || lv_weatherAppScr2 == NULL)
        return WEATHER_APP_PAGE::OTHER_PAGE;
    
    if (lv_scr_act() == lv_weatherAppScr1) {
        return WEATHER_APP_PAGE::CLOCK_PAGE;
    } else if (lv_scr_act() == lv_weatherAppScr2) {
        return WEATHER_APP_PAGE::CURVE_PAGE;
    }
    return WEATHER_APP_PAGE::OTHER_PAGE;
}

void DisplayCurve(short maxT[], short minT[])
{
    for (int Ti = 0; Ti < 7; ++Ti) {
        lv_tempHighSeries->y_points[Ti] = maxT[Ti] + 50; // 补偿50度
        lv_tempLowSeries->y_points[Ti] = minT[Ti] + 50; // 补偿50度
    }
    lv_chart_refresh(lv_weatherChart);
}

void DisplayWeather(struct WEATHER_STRUCT weatherInfo)
{

    lv_label_set_text(lv_cityLabel, weatherInfo.cityName.c_str());
    if (weatherInfo.cityName.length() > 6) {
        lv_obj_align(lv_cityLabel, LV_ALIGN_TOP_LEFT, 5, 15);
    } else {
        lv_obj_align(lv_cityLabel, LV_ALIGN_TOP_LEFT, 20, 15);
    }
    lv_label_set_text(lv_airQualityBtnLabel, AirQualityCh[weatherInfo.airQulity]);
    lv_img_set_src(lv_weatherIconImg, WeatherIconMap[weatherInfo.weatherCode]);
    lv_label_set_text_fmt(lv_weatherTextLabel, "最低气温%d°C, 最高气温%d°C, %s%d级.   ", weatherInfo.minTemp, weatherInfo.maxTemp, weatherInfo.windDir.c_str(), weatherInfo.windLevel);

    lv_bar_set_value(lv_tempBar, weatherInfo.temperature, LV_ANIM_ON);
    lv_label_set_text_fmt(lv_tempLabel, "%2d°C", weatherInfo.temperature);
    lv_bar_set_value(lv_humiBar, weatherInfo.humidity, LV_ANIM_ON);
    lv_label_set_text_fmt(lv_humiLabel, "%d%%", weatherInfo.humidity);
}
