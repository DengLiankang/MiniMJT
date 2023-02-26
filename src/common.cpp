#include "common.h"
#include "network.h"

IMU mpu;
SdCard tf;
Network g_network; // 网络连接
FlashFS gFlashCfg; // flash中的文件系统（替代原先的Preferences）
Display screen;    // 屏幕对象
Ambient ambLight;  // 光线传感器对象

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

#if GFX

#include <Arduino_GFX_Library.h>

Arduino_HWSPI *bus = new Arduino_HWSPI(TFT_DC /* DC */, TFT_CS /* CS */, TFT_SCLK, TFT_MOSI, TFT_MISO);
Arduino_ST7789 *tft = new Arduino_ST7789(bus, TFT_RST /* RST */, 3 /* rotation */, true /* IPS */, 240 /* width */,
                                         240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);

#else
#include <TFT_eSPI.h>
/*
TFT pins should be set in path/to/Arduino/libraries/TFT_eSPI/User_Setups/Setup24_ST7789.h
*/
TFT_eSPI *tft = new TFT_eSPI(SCREEN_HOR_RES, SCREEN_VER_RES);
#endif