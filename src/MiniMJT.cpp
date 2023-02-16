#include "driver/lv_port_fs.h"
#include "driver/lv_port_indev.h"

#include "common.h"
#include "sys/app_controller.h"

#include "app/app_conf.h"

#include <SPIFFS.h>
#include <esp32-hal-timer.h>
#include <esp32-hal.h>

static bool isCheckAction = false;

/*** Component objects ***/
ImuAction *act_info;           // 存放mpu6050返回的数据
AppController *app_controller; // APP控制器

TaskHandle_t handleTaskLvgl;

void TaskLvglUpdate(void *parameter)
{
    // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    for (;;) {
        AIO_LVGL_OPERATE_LOCK(lv_timer_handler();)
        vTaskDelay(5);
    }
}

TimerHandle_t xTimerAction = NULL;
void actionCheckHandle(TimerHandle_t xTimer)
{
    // 标志需要检测动作
    isCheckAction = true;
}

#if LV_USE_LOG
void minimjt_print(const char *buf)
{
    Serial.printf("%s", buf);
    Serial.flush();
}
#endif

void setup()
{
    Serial.begin(115200);

    Serial.println(F("\n==========MiniMJT version " MJT_VERSION "==========\n"));
    Serial.flush();
    // MAC ID可用作芯片唯一标识
    Serial.print(F("ChipID(EfuseMac): "));
    Serial.println(ESP.getEfuseMac());

    app_controller = new AppController(); // APP控制器

    // 需要放在Setup里初始化
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    // config_read(NULL, &g_cfg);   // 旧的配置文件读取方式
    app_controller->read_config(&app_controller->sys_cfg);
    app_controller->read_config(&app_controller->mpu_cfg);

    /*** Init screen ***/
    screen.init(app_controller->sys_cfg.rotation, app_controller->sys_cfg.backLight);

    app_controller->init();
    AIO_LVGL_OPERATE_LOCK(lv_timer_handler();)

    /*** Init ambient-light sensor ***/
    ambLight.init(ONE_TIME_H_RESOLUTION_MODE);

    /*** Init micro SD-Card ***/
    tf.init();

    lv_fs_fatfs_init();

#if LV_USE_LOG
    lv_log_register_print_cb(minimjt_print);
#endif /*LV_USE_LOG*/

    /*** Init IMU as input device ***/
    // lv_port_indev_init();

    mpu.init(app_controller->sys_cfg.mpu_order, app_controller->sys_cfg.auto_calibration_mpu,
             &app_controller->mpu_cfg); // 初始化比较耗时

    // 将APP"安装"到controller里
#if APP_WEATHER_USE
    app_controller->app_install(&weather_app);
#endif
#if APP_PICTURE_USE
    app_controller->app_install(&picture_app);
#endif
#if APP_MEDIA_PLAYER_USE
    app_controller->app_install(&media_app);
#endif
#if APP_SCREEN_SHARE_USE
    app_controller->app_install(&screen_share_app);
#endif
#if APP_FILE_MANAGER_USE
    app_controller->app_install(&file_manager_app);
#endif
#if APP_WEB_SERVER_USE
    app_controller->app_install(&server_app);
#endif
#if APP_IDEA_ANIM_USE
    app_controller->app_install(&idea_app);
#endif
#if APP_SETTING_USE
    app_controller->app_install(&settings_app);
#endif
#if APP_GAME_2048_USE
    app_controller->app_install(&game_2048_app);
#endif
#if APP_ANNIVERSARY_USE
    app_controller->app_install(&anniversary_app);
#endif
#if APP_HEARTBEAT_USE
    app_controller->app_install(&heartbeat_app, APP_TYPE_BACKGROUND);
#endif

    // 自启动APP
    app_controller->app_auto_start();

    // 先初始化一次动作数据 防空指针
    act_info = mpu.getAction();
    // 定义一个mpu6050的动作检测定时器
    xTimerAction = xTimerCreate("Action Check", 200 / portTICK_PERIOD_MS, pdTRUE, (void *)0, actionCheckHandle);
    xTimerStart(xTimerAction, 0);
}

void loop()
{
    screen.routine();

    if (isCheckAction) {
        isCheckAction = false;
        act_info = mpu.getAction();
    }
    app_controller->main_process(act_info); // 运行当前进程
    // Serial.println(ambLight.getLux() / 50.0);
}