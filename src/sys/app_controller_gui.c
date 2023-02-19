#include "sys/app_controller_gui.h"
// #include "lvgl.h"

// 必须定义为全局或者静态
static lv_obj_t *gAppCtrlScreen = NULL;
static lv_obj_t *gLoadingBar = NULL;
static lv_obj_t *gLoadingText = NULL;
static lv_obj_t *gBootingLogo = NULL;
static lv_obj_t *pre_app_image = NULL;
static lv_obj_t *pre_app_name = NULL;
static lv_obj_t *now_app_image = NULL;
static lv_obj_t *now_app_name = NULL;
const void *pre_img_path = NULL;

static lv_style_t gScreenStyle;
static lv_style_t gAppNameStyle;

LV_FONT_DECLARE(lv_font_montserrat_24);
LV_IMG_DECLARE(gBootingImg);

void AppCtrlScreenInit(void)
{
    lv_style_init(&gScreenStyle);
    lv_style_set_bg_color(&gScreenStyle, lv_color_hex(0x000000));
    lv_style_set_radius(&gScreenStyle, 0); // 设置控件圆角半径
    lv_style_set_border_width(&gScreenStyle, 0); // 设置边框宽度

    lv_style_init(&gAppNameStyle);
    lv_style_set_text_opa(&gAppNameStyle, LV_OPA_COVER);
    lv_style_set_text_color(&gAppNameStyle, lv_color_white());
    lv_style_set_text_font(&gAppNameStyle, &lv_font_montserrat_24);

    gAppCtrlScreen = lv_obj_create(NULL);
    lv_obj_add_style(gAppCtrlScreen, &gScreenStyle, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gAppCtrlScreen, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_size(gAppCtrlScreen, 240, 240);
    lv_obj_align(gAppCtrlScreen, LV_ALIGN_CENTER, 0, 0);
    lv_scr_load(gAppCtrlScreen);
}

void AppCtrlLoadingGuiInit(void)
{
    static lv_style_t style_bg;
    static lv_style_t style_indic;

    lv_style_init(&style_bg);
    lv_style_set_border_color(&style_bg, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_border_width(&style_bg, 1);
    lv_style_set_pad_all(&style_bg, 4); /*To make the indicator smaller*/
    lv_style_set_radius(&style_bg, 3);

    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_LIGHT_BLUE));
    lv_style_set_radius(&style_indic, 3);

    gLoadingBar = lv_bar_create(gAppCtrlScreen);
    lv_obj_remove_style_all(gLoadingBar);  /*To have a clean start*/
    lv_obj_add_style(gLoadingBar, &style_bg, LV_PART_MAIN);
    lv_obj_add_style(gLoadingBar, &style_indic, LV_PART_INDICATOR);
    lv_obj_set_size(gLoadingBar, 120, 10);
    lv_obj_align(gLoadingBar, LV_ALIGN_CENTER, 0, 20);
    lv_bar_set_value(gLoadingBar, 0, LV_ANIM_OFF);

    gLoadingText = lv_label_create(gAppCtrlScreen);
    lv_obj_set_style_text_color(gLoadingText, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(gLoadingText, LV_ALIGN_CENTER, 0, 40);
    lv_label_set_text(gLoadingText, "booting...");

    gBootingLogo = lv_img_create(gAppCtrlScreen);
    lv_img_set_src(gBootingLogo, &gBootingImg);
    lv_obj_align(gBootingLogo, LV_ALIGN_CENTER, 0, -40);
}

void AppCtrlLoadingDisplay(int progress, const char *text)
{
    int nowProgress = lv_bar_get_value(gLoadingBar);
    if (progress > nowProgress) {
        int animTime = (progress - nowProgress) * 50;
        lv_obj_set_style_anim_time(gLoadingBar, animTime, LV_PART_MAIN);
        lv_bar_set_value(gLoadingBar, progress, LV_ANIM_ON);
    }
    if (text != NULL) {
        lv_label_set_text(gLoadingText, text);
    }
}

void app_control_gui_release(void)
{
    if (NULL != gAppCtrlScreen) {
        lv_obj_clean(gAppCtrlScreen);
        gAppCtrlScreen = NULL;
    }
}

void display_app_scr_init(const void *src_img_path, const char *app_name)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == gAppCtrlScreen) {
        // 防止一些不适用lvgl的APP退出 造成画面在无其他动作情况下无法绘制更新
        lv_scr_load_anim(gAppCtrlScreen, LV_SCR_LOAD_ANIM_NONE, 300, 300, false);
        return;
    }

    lv_obj_clean(act_obj); // 清空此前页面
    pre_app_image = lv_img_create(gAppCtrlScreen);
    pre_img_path = src_img_path; // 保存历史
    lv_img_set_src(pre_app_image, src_img_path);
    lv_obj_align(pre_app_image, LV_ALIGN_CENTER, 0, -20);

    // 添加APP的名字
    pre_app_name = lv_label_create(gAppCtrlScreen);
    lv_obj_add_style(pre_app_name, &gAppNameStyle, LV_STATE_DEFAULT);
    // lv_label_set_recolor(pre_app_name, true); //先得使能文本重绘色功能
    lv_label_set_text(pre_app_name, app_name);
    lv_obj_align_to(pre_app_name, pre_app_image, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_scr_load_anim(gAppCtrlScreen, LV_SCR_LOAD_ANIM_NONE, 300, 300, false);
}

void app_control_display_scr(const void *src_img, const char *app_name, lv_scr_load_anim_t anim_type, bool force)
{
    // force为是否强制刷新页面 true为强制刷新
    if (true == force) {
        display_app_scr_init(src_img, app_name);
        return;
    }

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

    now_app_image = lv_img_create(gAppCtrlScreen);
    lv_img_set_src(now_app_image, src_img);
    lv_obj_align(now_app_image, LV_ALIGN_CENTER, 0, -20);
    // 添加APP的名字
    now_app_name = lv_label_create(gAppCtrlScreen);
    lv_obj_add_style(now_app_name, &gAppNameStyle, LV_STATE_DEFAULT);
    // lv_label_set_recolor(now_app_name, true); //先得使能文本重绘色功能
    lv_label_set_text(now_app_name, app_name);
    // 删除原先的APP name
    lv_obj_del(pre_app_name);
    pre_app_name = now_app_name;
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
    lv_anim_set_var(&pre_app, pre_app_image);
    lv_anim_set_values(&pre_app, old_start_x, old_end_x);
    duration = lv_anim_speed_to_time(400, old_start_x, old_end_x); // 计算时间
    lv_anim_set_time(&pre_app, duration);
    lv_anim_set_path_cb(&pre_app, lv_anim_path_linear); // 设置一个动画的路径

    lv_anim_start(&now_app);
    lv_anim_start(&pre_app);
    ANIEND_WAIT;

    lv_obj_del(pre_app_image); // 删除原先的图像
    pre_app_image = now_app_image;
}