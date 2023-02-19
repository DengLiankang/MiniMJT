#ifndef APP_CONTROLLER_GUI_H
#define APP_CONTROLLER_GUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#define ANIEND_WAIT while (lv_anim_count_running())

void AppCtrlScreenInit(void);
void AppCtrlLoadingGuiInit(void);
void AppCtrlLoadingDisplay(int progress, const char *text);
void app_control_gui_release(void);
void display_app_scr_release(void);
void display_app_scr_init(const void *src_img, const char *app_name);
void app_control_display_scr(const void *src_img, const char *app_name, lv_scr_load_anim_t anim_type, bool force);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif