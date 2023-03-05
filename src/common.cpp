#include "common.h"

IMU mpu;
SdCard g_tfCard;
Network g_network; // 网络连接
FlashFs g_flashFs; // flash中的文件系统
Display screen;    // 屏幕对象
TFT_eSPI *tft;

TaskHandle_t gTaskLvglHandle;

// lvgl handle的锁
SemaphoreHandle_t lvgl_mutex = xSemaphoreCreateMutex();

void LvglTask(void *pvParameters)
{
    while (!lv_is_initialized()) {
        delay(1);
    }
    while (1) {
        MJT_LVGL_OPERATE_LOCK(lv_timer_handler());
        delay(1);
    }
}

void InitLvglTaskSetup(const char *name)
{
    xTaskCreate(LvglTask, name, 8 * 1024, NULL, TASK_LVGL_PRIORITY, &gTaskLvglHandle);
}

void DeleteLvglTask(void)
{
    vTaskDelete(gTaskLvglHandle);
}

boolean doDelayMillisTime(unsigned long interval, unsigned long *previousMillis, boolean state)
{
    unsigned long currentMillis = GET_SYS_MILLIS();
    if (currentMillis - *previousMillis >= interval) {
        *previousMillis = currentMillis;
        state = !state;
    }
    return state;
}

void ParseParam(char *info, int argc, char **argv)
{
    // cnt记录解析到第几个参数
    for (int cnt = 0; cnt < argc; ++cnt) {
        argv[cnt] = info;
        while (*info != '\n') {
            ++info;
        }
        *info = 0;
        ++info;
    }
}
