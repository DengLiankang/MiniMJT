#include "weather_gui.h"
#include "weather_image.h"

#include "lvgl.h"

LV_FONT_DECLARE(lv_font_ibmplex_115);
LV_FONT_DECLARE(lv_font_ibmplex_64);
LV_FONT_DECLARE(ch_yahei_font_20);

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
static const char WeekDayCh[7][4] = {"日", "一", "二", "三", "四", "五", "六"};
static const char AirQualityCh[6][10] = {"优", "良", "轻度", "中度", "重度", "严重"};

void weather_gui_init(void)
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

    // weather clock scr
    lv_weatherAppScr1 = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(lv_weatherAppScr1, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_size(lv_weatherAppScr1, 240, 240);
    lv_obj_align(lv_weatherAppScr1, LV_ALIGN_CENTER, 0, 0);

    lv_weatherIconImg = lv_img_create(lv_weatherAppScr1);
    lv_img_set_src(lv_weatherIconImg, WeatherIconMap[0]);

    lv_cityLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_cityLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_label_set_recolor(lv_cityLabel, true);
    lv_label_set_text(lv_cityLabel, "上海");

    lv_airQualityBtn = lv_btn_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_airQualityBtn, &lv_airQualityBtnStyle, LV_STATE_DEFAULT);
    lv_obj_set_pos(lv_airQualityBtn, 75, 15);
    lv_obj_set_size(lv_airQualityBtn, 50, 25);
    lv_obj_set_style_bg_color(lv_airQualityBtn, lv_palette_main(LV_PALETTE_ORANGE), LV_STATE_DEFAULT);

    lv_airQualityBtnLabel = lv_label_create(lv_airQualityBtn);
    lv_obj_add_style(lv_airQualityBtnLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_obj_align(lv_airQualityBtnLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(lv_airQualityBtnLabel, AirQualityCh[0]);

    lv_weatherTextLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_weatherTextLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_obj_set_size(lv_weatherTextLabel, 120, 30);
    lv_label_set_long_mode(lv_weatherTextLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text_fmt(lv_weatherTextLabel, "最低气温%d°C, 最高气温%d°C, %s%d级.   ", 15, 20, "西北风", 0);

    lv_clockLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_clockLabel, &lv_clockNumStyle, LV_STATE_DEFAULT);
    lv_label_set_recolor(lv_clockLabel, true);
    lv_label_set_text_fmt(lv_clockLabel, "%02d#ffa500 %02d#", 10, 52);

    lv_secondLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_secondLabel, &lv_secondNumStyle, LV_STATE_DEFAULT);
    lv_label_set_recolor(lv_secondLabel, true);
    lv_label_set_text_fmt(lv_secondLabel, "%02d", 00);

    lv_dateLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_dateLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(lv_dateLabel, "%2d月%2d日   周%s", 11, 23, WeekDayCh[1]);

    lv_tempImg = lv_img_create(lv_weatherAppScr1);
    lv_img_set_src(lv_tempImg, &tempIcon);
    lv_tempBar = lv_bar_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_tempBar, &lv_humiBarStyle, LV_STATE_DEFAULT);
    lv_bar_set_range(lv_tempBar, -50, 50); // 设置进度条表示的温度为-50~50
    lv_obj_set_size(lv_tempBar, 60, 12);
    lv_obj_set_style_bg_color(lv_tempBar, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
    lv_bar_set_value(lv_tempBar, 10, LV_ANIM_ON);
    lv_tempLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_tempLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(lv_tempLabel, "%2d°C", 18);

    lv_humiImg = lv_img_create(lv_weatherAppScr1);
    lv_img_set_src(lv_humiImg, &humiIcon);
    lv_humiBar = lv_bar_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_humiBar, &lv_humiBarStyle, LV_STATE_DEFAULT);
    lv_bar_set_range(lv_humiBar, 0, 100);
    lv_obj_set_size(lv_humiBar, 60, 12);
    lv_obj_set_style_bg_color(lv_humiBar, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);
    lv_bar_set_value(lv_humiBar, 49, LV_ANIM_ON);
    lv_humiLabel = lv_label_create(lv_weatherAppScr1);
    lv_obj_add_style(lv_humiLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_label_set_text(lv_humiLabel, "50%");

    // 太空人图标
    lv_spaceManImg = lv_img_create(lv_weatherAppScr1);
    lv_img_set_src(lv_spaceManImg, SpaceManImgMap[0]);

    // 绘制图形
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

    // weather curve scr
    lv_weatherAppScr2 = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(lv_weatherAppScr2, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_size(lv_weatherAppScr2, 240, 240);
    lv_obj_align(lv_weatherAppScr2, LV_ALIGN_CENTER, 0, 0);

    lv_weatherChartLabel = lv_label_create(lv_weatherAppScr2);
    lv_obj_add_style(lv_weatherChartLabel, &lv_chFontStyle, LV_STATE_DEFAULT);
    lv_label_set_recolor(lv_weatherChartLabel, true);
    lv_label_set_text(lv_weatherChartLabel, "查看更多天气");

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

void display_curve_init(lv_scr_load_anim_t anim_type)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == lv_weatherAppScr2)
        return;

    weather_gui_release();

    if (LV_SCR_LOAD_ANIM_NONE != anim_type) {
        lv_scr_load_anim(lv_weatherAppScr2, anim_type, 300, 300, false);
    } else {
        lv_scr_load(lv_weatherAppScr2);
    }
}

void display_curve(short maxT[], short minT[], lv_scr_load_anim_t anim_type)
{
    display_curve_init(anim_type);
    for (int Ti = 0; Ti < 7; ++Ti) {
        lv_tempHighSeries->y_points[Ti] = maxT[Ti] + 50; // 补偿50度
    }
    for (int Ti = 0; Ti < 7; ++Ti) {
        lv_tempLowSeries->y_points[Ti] = minT[Ti] + 50; // 补偿50度
    }
    lv_chart_refresh(lv_weatherChart);
}

void display_weather_init(lv_scr_load_anim_t anim_type)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == lv_weatherAppScr1)
        return;

    weather_gui_release();

    // if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    // {
    //     lv_scr_load_anim(lv_weatherAppScr1, anim_type, 300, 300, false);
    // }
    // else
    // {
    // lv_scr_load(lv_weatherAppScr1);
    // }
}

void display_weather(struct Weather weaInfo, lv_scr_load_anim_t anim_type)
{
    display_weather_init(anim_type);

    lv_label_set_text(lv_cityLabel, weaInfo.cityname);
    if (strlen(weaInfo.cityname) > 6) {
        lv_obj_align(lv_cityLabel, LV_ALIGN_TOP_LEFT, 5, 15);
    } else {
        lv_obj_align(lv_cityLabel, LV_ALIGN_TOP_LEFT, 20, 15);
    }
    lv_label_set_text(lv_airQualityBtnLabel, AirQualityCh[weaInfo.airQulity]);
    lv_img_set_src(lv_weatherIconImg, WeatherIconMap[weaInfo.weather_code]);
    // 下面这行代码可能会出错
    lv_label_set_text_fmt(lv_weatherTextLabel, "最低气温%d°C, 最高气温%d°C, %s%d 级.   ", weaInfo.minTemp,
                          weaInfo.maxTemp, weaInfo.windDir, weaInfo.windLevel);

    lv_bar_set_value(lv_tempBar, weaInfo.temperature, LV_ANIM_ON);
    lv_label_set_text_fmt(lv_tempLabel, "%2d°C", weaInfo.temperature);
    lv_bar_set_value(lv_humiBar, weaInfo.humidity, LV_ANIM_ON);
    lv_label_set_text_fmt(lv_humiLabel, "%d%%", weaInfo.humidity);

    // // 绘制图形
    // lv_obj_align(lv_weatherIconImg, NULL, LV_ALIGN_TOP_RIGHT, -10, 10);
    // lv_obj_align(lv_cityLabel, NULL, LV_ALIGN_TOP_LEFT, 20, 15);
    // lv_obj_align(lv_weatherTextLabel, NULL, LV_ALIGN_TOP_LEFT, 0, 50);
    // lv_obj_align(lv_tempImg, NULL, LV_ALIGN_LEFT_MID, 10, 70);
    // lv_obj_align(lv_tempBar, NULL, LV_ALIGN_LEFT_MID, 35, 70);
    // lv_obj_align(lv_tempLabel, NULL, LV_ALIGN_LEFT_MID, 100, 70);
    // lv_obj_align(lv_humiImg, NULL, LV_ALIGN_LEFT_MID, 0, 100);
    // lv_obj_align(lv_humiBar, NULL, LV_ALIGN_LEFT_MID, 35, 100);
    // lv_obj_align(lv_humiLabel, NULL, LV_ALIGN_LEFT_MID, 100, 100);
    // lv_obj_align(lv_spaceManImg, NULL, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    if (LV_SCR_LOAD_ANIM_NONE != anim_type) {
        lv_scr_load_anim(lv_weatherAppScr1, anim_type, 300, 300, false);
    } else {
        lv_scr_load(lv_weatherAppScr1);
    }
}

void display_time(struct TimeStr timeInfo, lv_scr_load_anim_t anim_type)
{
    display_weather_init(anim_type);
    lv_label_set_text_fmt(lv_clockLabel, "%02d#ffa500 %02d#", timeInfo.hour, timeInfo.minute);
    lv_label_set_text_fmt(lv_secondLabel, "%02d", timeInfo.second);
    lv_label_set_text_fmt(lv_dateLabel, "%2d月%2d日   周%s", timeInfo.month, timeInfo.day, WeekDayCh[timeInfo.weekday]);

    // lv_obj_align(lv_clockLabel, NULL, LV_ALIGN_LEFT_MID, 0, 10);
    // lv_obj_align(lv_secondLabel, NULL, LV_ALIGN_LEFT_MID, 165, 9);
    // lv_obj_align(lv_dateLabel, NULL, LV_ALIGN_LEFT_MID, 10, 32);

    // if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    // {
    //     lv_scr_load_anim(lv_weatherAppScr1, anim_type, 300, 300, false);
    // }
    // else
    // {
    //     lv_scr_load(lv_weatherAppScr1);
    // }
}

void weather_gui_release(void)
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

void weather_gui_del(void)
{
    weather_gui_release();

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&lv_chFontStyle);
    // lv_style_reset(&lv_secondNumStyle);
    // lv_style_reset(&lv_clockNumStyle);
    // lv_style_reset(&lv_airQualityBtnStyle);
    // lv_style_reset(&lv_humiBarStyle);
}

void display_space(void)
{
    static int _spaceIndex = 0;
    if (NULL != lv_weatherAppScr1 && lv_scr_act() == lv_weatherAppScr1) {
        lv_img_set_src(lv_spaceManImg, SpaceManImgMap[_spaceIndex]);
        _spaceIndex = (_spaceIndex + 1) % 10;
    }
}

int airQulityLevel(int q)
{
    if (q < 50) {
        return 0;
    } else if (q < 100) {
        return 1;
    } else if (q < 150) {
        return 2;
    } else if (q < 200) {
        return 3;
    } else if (q < 300) {
        return 4;
    }
    return 5;
}