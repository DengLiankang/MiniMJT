#ifndef APP_ANNIVERSARY_GUI_H
#define APP_ANNIVERSARY_GUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "time.h"

void anniversary_gui_init(void);
void display_anniversary(const char *file_name, lv_scr_load_anim_t anim_type, struct tm *target_date,
                         int anniversary_day_count, const char *event_name);
void anniversary_gui_display_date(struct tm *target, int anniversary_day_count, const char *event_name);
void anniversary_gui_set_txtlabel(char *txt);
void anniversary_gui_del(void);
void anniversary_obj_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
extern const lv_img_dsc_t app_anniversary;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif