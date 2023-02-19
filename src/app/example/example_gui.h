#ifndef APP_EXAMPLE_GUI_H
#define APP_EXAMPLE_GUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

void example_gui_init(void);
void display_example(const char *file_name, lv_scr_load_anim_t anim_type);
void example_gui_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
extern const lv_img_dsc_t app_example;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif