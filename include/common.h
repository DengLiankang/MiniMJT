#ifndef _COMMON_H_
#define _COMMON_H_

#include "Arduino.h"
#include "driver/display.h"
#include "driver/flash_fs.h"
#include "driver/imu.h"
#include "driver/sd_card.h"
#include "driver/network.h"
#include <TFT_eSPI.h>

#define MJT_VERSION "2.1.13"

// SD_Card
#define SD_SCK 14
#define SD_MISO 26
#define SD_MOSI 13
#define SD_SS 15

// MUP6050
#define IMU_I2C_SDA 32
#define IMU_I2C_SCL 33

// 屏幕尺寸
#define SCREEN_HOR_RES 240 // 水平
#define SCREEN_VER_RES 240 // 竖直

// TFT屏幕接口
#define LCD_BL_PIN 5

#define LCD_BL_PWM_CHANNEL 0

// 优先级定义(数值越小优先级越高)
// 最高为 configMAX_PRIORITIES-1
#define TASK_LVGL_PRIORITY 2 // LVGL的页面优先级

#define MAX_CFG_INFO_LENGTH 128

struct SysUtilConfig {
    String ssid[3];
    String password[3];
    String autoStartAppName; // 开机自启的APP名字
    uint8_t powerMode;       // 功耗模式（0为节能模式 1为性能模式）
    uint8_t backlight;       // 屏幕亮度（1-100）
    uint8_t rotation;        // 屏幕旋转方向
    // imu
    uint8_t imuAutoCalibration; // 是否自动校准陀螺仪 0关闭自动校准 1打开自动校准
    uint8_t imuOrder;           // 操作方向
    struct ImuOffsetConfig imuOffsets;
};

extern SdCard g_tfCard;
extern FlashFs g_flashFs; // flash中的文件系统
extern TFT_eSPI *tft;
extern SemaphoreHandle_t lvgl_mutex; // lvgl 操作的锁

// LVGL操作的安全宏（避免脏数据）
#define MJT_LVGL_OPERATE_LOCK(CODE)                                                                                    \
    {                                                                                                                  \
        if (pdTRUE == xSemaphoreTake(lvgl_mutex, portMAX_DELAY)) {                                                     \
            CODE;                                                                                                      \
            xSemaphoreGive(lvgl_mutex);                                                                                \
        }                                                                                                              \
    }

void InitLvglTaskSetup(const char *name);
void DeleteLvglTask(void);
boolean DoDelayMillisTime(unsigned long interval, unsigned long *previousMillis);
void ParseParam(char *src, int argc, char **dst);

#endif