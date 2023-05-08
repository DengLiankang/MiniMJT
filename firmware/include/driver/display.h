#ifndef DISPLAY_H
#define DISPLAY_H

#include <lvgl.h>

class Display
{
public:
    void init(uint8_t rotation, uint8_t backlight);
    void routine();
    void setBackLight(float);
};

#endif
