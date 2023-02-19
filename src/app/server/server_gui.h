#ifndef APP_SERVER_GUI_H
#define APP_SERVER_GUI_H

// #include "lv_port_indev.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

void server_gui_init(void);
void display_setting_init(void);
void display_setting(const char *title, const char *domain, const char *info, const char *ap_ip,
                     lv_scr_load_anim_t anim_type);
void setting_gui_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
extern const lv_img_dsc_t app_server;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif