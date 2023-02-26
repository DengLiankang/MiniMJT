#include "sys/app_controller_gui.h"
// #include "lvgl.h"

// 必须定义为全局或者静态
// Loading
static lv_obj_t *gLvglScrLoading = NULL;
static lv_obj_t *gLvglBarLoading = NULL;
static lv_obj_t *gLvglLabelLoading = NULL;
static lv_obj_t *gLvglImgBooting = NULL;

// Menu
static struct AppCtrlMenuPage gAppMenuPage1;
static struct AppCtrlMenuPage gAppMenuPage2;
static struct AppCtrlMenuPage *gNextMenuPage;

static lv_obj_t *now_app_image = NULL;
static lv_obj_t *now_app_name = NULL;
const void *pre_img_path = NULL;

LV_IMG_DECLARE(LvImgBooting);
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

    gLvglScrLoading = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(gLvglScrLoading, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_size(gLvglScrLoading, 240, 240);
    lv_obj_align(gLvglScrLoading, LV_ALIGN_CENTER, 0, 0);

    gLvglBarLoading = lv_bar_create(gLvglScrLoading);
    lv_obj_remove_style_all(gLvglBarLoading);  /*To have a clean start*/
    lv_obj_add_style(gLvglBarLoading, &BarBoderStyle, LV_PART_MAIN);
    lv_obj_add_style(gLvglBarLoading, &BarIndicStyle, LV_PART_INDICATOR);
    lv_obj_set_size(gLvglBarLoading, 120, 10);
    lv_obj_align(gLvglBarLoading, LV_ALIGN_CENTER, 0, 20);
    lv_bar_set_value(gLvglBarLoading, 0, LV_ANIM_OFF);

    gLvglLabelLoading = lv_label_create(gLvglScrLoading);
    lv_obj_set_style_text_color(gLvglLabelLoading, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_recolor(gLvglLabelLoading, true);
    lv_obj_align(gLvglLabelLoading, LV_ALIGN_CENTER, 0, 40);
    lv_label_set_text(gLvglLabelLoading, "booting...");

    gLvglImgBooting = lv_img_create(gLvglScrLoading);
    lv_img_set_src(gLvglImgBooting, &LvImgBooting);
    lv_obj_align(gLvglImgBooting, LV_ALIGN_CENTER, 0, -40);

    lv_scr_load(gLvglScrLoading);
}

void AppCtrlLoadingDisplay(int progress, const char *text, bool wait)
{
    if (lv_scr_act() != gLvglScrLoading) {
        return;
    }
    int nowProgress = lv_bar_get_value(gLvglBarLoading);
    if (progress > nowProgress) {
        int animTime = (progress - nowProgress) * 50;
        lv_obj_set_style_anim_time(gLvglBarLoading, animTime, LV_PART_MAIN);
        lv_bar_set_value(gLvglBarLoading, progress, LV_ANIM_ON);
    }
    if (wait) {
        ANIEND_WAIT;
    }
    if (text != NULL) {
        lv_label_set_text(gLvglLabelLoading, text);
    }
}

void AppCtrlMenuGuiInit(void)
{
    static lv_style_t appNameStyle;

    lv_style_init(&appNameStyle);
    lv_style_set_text_opa(&appNameStyle, LV_OPA_COVER);
    lv_style_set_text_color(&appNameStyle, lv_color_white());
    lv_style_set_text_font(&appNameStyle, &lv_font_montserrat_24);

    gAppMenuPage1.appMenuScr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(gAppMenuPage1.appMenuScr, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_size(gAppMenuPage1.appMenuScr, 240, 240);
    lv_obj_align(gAppMenuPage1.appMenuScr, LV_ALIGN_CENTER, 0, 0);

    gAppMenuPage1.appImg = lv_img_create(gAppMenuPage1.appMenuScr);
    lv_obj_align(gAppMenuPage1.appImg, LV_ALIGN_CENTER, 0, -20);

    gAppMenuPage1.appName = lv_label_create(gAppMenuPage1.appMenuScr);
    lv_obj_add_style(gAppMenuPage1.appName, &appNameStyle, LV_STATE_DEFAULT);
    lv_obj_align(gAppMenuPage1.appName, LV_ALIGN_BOTTOM_MID, 0, -20);

    gAppMenuPage2.appMenuScr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(gAppMenuPage2.appMenuScr, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_size(gAppMenuPage2.appMenuScr, 240, 240);
    lv_obj_align(gAppMenuPage2.appMenuScr, LV_ALIGN_CENTER, 0, 0);

    gAppMenuPage2.appImg = lv_img_create(gAppMenuPage2.appMenuScr);
    lv_obj_align(gAppMenuPage2.appImg, LV_ALIGN_CENTER, 0, -20);

    gAppMenuPage2.appName = lv_label_create(gAppMenuPage2.appMenuScr);
    lv_obj_add_style(gAppMenuPage2.appName, &appNameStyle, LV_STATE_DEFAULT);
    lv_obj_align(gAppMenuPage2.appName, LV_ALIGN_BOTTOM_MID, 0, -20);

    gAppMenuPage1.nextPage = &gAppMenuPage2;
    gAppMenuPage2.nextPage = &gAppMenuPage1;
    gNextMenuPage = &gAppMenuPage1;
}

void AppCtrlMunuDisplay(const void *appImg, const char *appName, lv_scr_load_anim_t anim, bool delPre)
{
    lv_img_set_src(gNextMenuPage->appImg, appImg);
    lv_label_set_text(gNextMenuPage->appName, appName);

    lv_scr_load_anim(gNextMenuPage->appMenuScr, anim, 250, 250, delPre);

    gNextMenuPage = gNextMenuPage->nextPage;
}

void app_control_gui_release(void)
{
    if (NULL != gAppMenuPage1.appMenuScr) {
        lv_obj_clean(gAppMenuPage1.appMenuScr);
        gAppMenuPage1.appMenuScr = NULL;
    }
}

void app_control_display_scr(const void *src_img, const char *app_name, lv_scr_load_anim_t anim_type, bool force)
{
    // force为是否强制刷新页面 true为强制刷新
    // if (true == force) {
    //     display_app_scr_init(src_img, app_name);
    //     return;
    // }

    if (src_img == pre_img_path) {
        return;
    }

    pre_img_path = src_img;
    int now_start_x;
    int now_end_x;
    int old_start_x;
    int old_end_x;

    if (LV_SCR_LOAD_ANIM_MOVE_LEFT == anim_type) {
        // 120为半个屏幕大小 应用图标规定是128，一半刚好是64
        now_start_x = -120 - 64;
        now_end_x = 0;
        old_start_x = 0;
        old_end_x = 120 + 64;
    } else {
        // 120为半个屏幕大小 应用图标规定是128，一半刚好是64
        now_start_x = 120 + 64;
        now_end_x = 0;
        old_start_x = 0;
        old_end_x = -120 - 64;
    }

    now_app_image = lv_img_create(gAppMenuPage1.appMenuScr);
    lv_img_set_src(now_app_image, src_img);
    lv_obj_align(now_app_image, LV_ALIGN_CENTER, 0, -20);
    // 添加APP的名字
    now_app_name = lv_label_create(gAppMenuPage1.appMenuScr);
    // lv_obj_add_style(now_app_name, &gAppNameStyle, LV_STATE_DEFAULT);
    // lv_label_set_recolor(now_app_name, true); //先得使能文本重绘色功能
    lv_label_set_text(now_app_name, app_name);
    // 删除原先的APP name
    lv_obj_del(gAppMenuPage1.appName);
    gAppMenuPage1.appName = now_app_name;
    lv_obj_align_to(now_app_name, now_app_image, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    static lv_anim_t now_app;
    lv_anim_init(&now_app);
    lv_anim_set_exec_cb(&now_app, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_var(&now_app, now_app_image);
    lv_anim_set_values(&now_app, now_start_x, now_end_x);
    uint32_t duration = lv_anim_speed_to_time(400, now_start_x, now_end_x); // 计算时间
    lv_anim_set_time(&now_app, duration);
    lv_anim_set_path_cb(&now_app, lv_anim_path_linear); // 设置一个动画的路径

    static lv_anim_t pre_app;
    lv_anim_init(&pre_app);
    lv_anim_set_exec_cb(&pre_app, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_var(&pre_app, gAppMenuPage1.appImg);
    lv_anim_set_values(&pre_app, old_start_x, old_end_x);
    duration = lv_anim_speed_to_time(400, old_start_x, old_end_x); // 计算时间
    lv_anim_set_time(&pre_app, duration);
    lv_anim_set_path_cb(&pre_app, lv_anim_path_linear); // 设置一个动画的路径

    lv_anim_start(&now_app);
    lv_anim_start(&pre_app);
    ANIEND_WAIT;

    lv_obj_del(gAppMenuPage1.appImg); // 删除原先的图像
    gAppMenuPage1.appImg = now_app_image;
}