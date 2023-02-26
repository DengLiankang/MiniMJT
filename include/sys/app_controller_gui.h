#ifndef APP_CONTROLLER_GUI_H
#define APP_CONTROLLER_GUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#define ANIEND_WAIT                     \
    do {                                \
        lv_timer_handler();             \
        delay(1);                       \
    } while (lv_anim_count_running())

struct AppCtrlMenuPage {
    lv_obj_t *appMenuScr;
    lv_obj_t *appImg;
    lv_obj_t *appName;
    struct AppCtrlMenuPage *nextPage;
};

void AppCtrlMenuGuiInit(void);
void AppCtrlLoadingGuiInit(void);
void AppCtrlLoadingDisplay(int progress, const char *text, bool wait);
void AppCtrlMunuDisplay(const void *appImg, const char *appName, lv_scr_load_anim_t anim, bool delPre);
void display_app_scr_release(void);
void display_app_scr_init(const void *src_img, const char *app_name);
void app_control_display_scr(const void *src_img, const char *app_name, lv_scr_load_anim_t anim_type, bool force);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif