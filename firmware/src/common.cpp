#include "common.h"

FlashFs g_flashFs; // flash中的文件系统
SdCard g_tfCard;
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

boolean DoDelayMillisTime(unsigned long interval, unsigned long *previousMillis)
{
    unsigned long currentMillis = millis();
    if (currentMillis - *previousMillis >= interval) {
        *previousMillis = currentMillis;
        return true;
    }
    return false;
}

/// @brief 通过换行符解析每一行的参数，使用本函数一定要小心内存溢出
/// @param src 参数来源
/// @param argc 总共要解析的参数数量
/// @param dst 存放参数的数据结构
void ParseParam(char *src, int argc, char **dst)
{
    // cnt记录解析到第几个参数
    for (int cnt = 0; cnt < argc; ++cnt) {
        dst[cnt] = src;
        while (*src != '\n') {
            ++src;
        }
        *src = 0;
        ++src;
    }
}
