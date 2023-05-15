#include "sys/app_controller_gui.h"

// 必须定义为全局或者静态
// Loading
static lv_obj_t *gLv_scrLoading = NULL;
static lv_obj_t *gLv_barLoading = NULL;
static lv_obj_t *gLv_labelLoading = NULL;
static lv_obj_t *gLv_imgBooting = NULL;

// Menu
static struct AppCtrlMenuPage g_appMenuPage1;
static struct AppCtrlMenuPage g_appMenuPage2;
static struct AppCtrlMenuPage *g_nextMenuPage;

LV_IMG_DECLARE(minimjt_logo);
LV_IMG_DECLARE(default_app_icon);
LV_FONT_DECLARE(lv_font_montserrat_24);

void AppCtrlLoadingGuiInit(void)
{
    static lv_style_t BarBoderStyle;
    static lv_style_t BarIndicStyle;

    lv_style_init(&BarBoderStyle);
    lv_style_set_border_color(&BarBoderStyle, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_border_width(&BarBoderStyle, 1);
    lv_style_set_pad_all(&BarBoderStyle, 4); /*To make the indicator smaller*/
    lv_style_set_radius(&BarBoderStyle, 3);

    lv_style_init(&BarIndicStyle);
    lv_style_set_bg_opa(&BarIndicStyle, LV_OPA_COVER);
    lv_style_set_bg_color(&BarIndicStyle, lv_palette_main(LV_PALETTE_LIGHT_BLUE));
    lv_style_set_radius(&BarIndicStyle, 3);

    gLv_scrLoading = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(gLv_scrLoading, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_size(gLv_scrLoading, 240, 240);
    lv_obj_align(gLv_scrLoading, LV_ALIGN_CENTER, 0, 0);

    gLv_barLoading = lv_bar_create(gLv_scrLoading);
    lv_obj_remove_style_all(gLv_barLoading); /*To have a clean start*/
    lv_obj_add_style(gLv_barLoading, &BarBoderStyle, LV_PART_MAIN);
    lv_obj_add_style(gLv_barLoading, &BarIndicStyle, LV_PART_INDICATOR);
    lv_obj_set_size(gLv_barLoading, 120, 10);
    lv_obj_align(gLv_barLoading, LV_ALIGN_CENTER, 0, 20);
    lv_bar_set_value(gLv_barLoading, 0, LV_ANIM_OFF);

    gLv_labelLoading = lv_label_create(gLv_scrLoading);
    lv_obj_set_style_text_color(gLv_labelLoading, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_recolor(gLv_labelLoading, true);
    lv_obj_align(gLv_labelLoading, LV_ALIGN_CENTER, 0, 40);
    lv_label_set_text(gLv_labelLoading, "booting...");

    gLv_imgBooting = lv_img_create(gLv_scrLoading);
    lv_img_set_src(gLv_imgBooting, &minimjt_logo);
    lv_obj_align(gLv_imgBooting, LV_ALIGN_CENTER, 0, -40);

    lv_scr_load(gLv_scrLoading);
}

void AppCtrlLoadingDisplay(int progress, const char *text, bool wait)
{
    if (lv_scr_act() != gLv_scrLoading) {
        return;
    }
    int nowProgress = lv_bar_get_value(gLv_barLoading);
    if (progress > nowProgress) {
        int animTime = (progress - nowProgress) * 50;
        lv_obj_set_style_anim_time(gLv_barLoading, animTime, LV_PART_MAIN);
        lv_bar_set_value(gLv_barLoading, progress, LV_ANIM_ON);
    }
    if (wait) {
        ANIEND_WAIT;
    }
    if (text != NULL) {
        lv_label_set_text(gLv_labelLoading, text);
    }
}

void AppCtrlMenuGuiInit(void)
{
    static lv_style_t appNameStyle;

    lv_style_init(&appNameStyle);
    lv_style_set_text_opa(&appNameStyle, LV_OPA_COVER);
    lv_style_set_text_color(&appNameStyle, lv_color_white());
    lv_style_set_text_font(&appNameStyle, &lv_font_montserrat_24);

    g_appMenuPage1.appMenuScr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(g_appMenuPage1.appMenuScr, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_size(g_appMenuPage1.appMenuScr, 240, 240);
    lv_obj_align(g_appMenuPage1.appMenuScr, LV_ALIGN_CENTER, 0, 0);

    g_appMenuPage1.appImg = lv_img_create(g_appMenuPage1.appMenuScr);
    lv_obj_align(g_appMenuPage1.appImg, LV_ALIGN_CENTER, 0, -20);

    g_appMenuPage1.appName = lv_label_create(g_appMenuPage1.appMenuScr);
    lv_obj_add_style(g_appMenuPage1.appName, &appNameStyle, LV_STATE_DEFAULT);
    lv_obj_align(g_appMenuPage1.appName, LV_ALIGN_BOTTOM_MID, 0, -20);

    g_appMenuPage2.appMenuScr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(g_appMenuPage2.appMenuScr, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_size(g_appMenuPage2.appMenuScr, 240, 240);
    lv_obj_align(g_appMenuPage2.appMenuScr, LV_ALIGN_CENTER, 0, 0);

    g_appMenuPage2.appImg = lv_img_create(g_appMenuPage2.appMenuScr);
    lv_obj_align(g_appMenuPage2.appImg, LV_ALIGN_CENTER, 0, -20);

    g_appMenuPage2.appName = lv_label_create(g_appMenuPage2.appMenuScr);
    lv_obj_add_style(g_appMenuPage2.appName, &appNameStyle, LV_STATE_DEFAULT);
    lv_obj_align(g_appMenuPage2.appName, LV_ALIGN_BOTTOM_MID, 0, -20);

    g_appMenuPage1.nextPage = &g_appMenuPage2;
    g_appMenuPage2.nextPage = &g_appMenuPage1;
    g_nextMenuPage = &g_appMenuPage1;
}

void AppCtrlMenuDisplay(const char *appName, lv_scr_load_anim_t anim, bool delPre)
{
    String appNameStr(appName);
    appNameStr.toLowerCase();
    String appIconPath = LV_FS_FATFS_LETTER + ":/system/" + appNameStr + "/logo.bin";
    if (g_tfCard.FileExists(appIconPath.c_str()) == true) {
        lv_img_set_src(g_nextMenuPage->appImg, appIconPath.c_str());
    }
    else {
        lv_img_set_src(g_nextMenuPage->appImg, &default_app_icon);
    }
    lv_label_set_text(g_nextMenuPage->appName, appName);

    lv_scr_load_anim(g_nextMenuPage->appMenuScr, anim, 500, 500, delPre);

    ANIEND_WAIT;

    g_nextMenuPage = g_nextMenuPage->nextPage;
}
