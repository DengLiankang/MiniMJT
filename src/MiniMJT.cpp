#include "app/app_conf.h"
#include "common.h"
#include "driver/lv_port_indev.h"
#include "sys/app_controller.h"
#include <SPIFFS.h>

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

    InitLvglTaskSetup("LvglTask");

    gAppController = new AppController(); // APP控制器

    gAppController->init();

    // 将APP"安装"到controller里
#if APP_WEATHER_USE
    gAppController->app_install(&weather_app);
#endif
#if APP_PICTURE_USE
    gAppController->app_install(&picture_app);
#endif
#if APP_MEDIA_PLAYER_USE
    gAppController->app_install(&media_app);
#endif
#if APP_SCREEN_SHARE_USE
    gAppController->app_install(&screen_share_app);
#endif
#if APP_FILE_MANAGER_USE
    gAppController->app_install(&file_manager_app);
#endif
#if APP_WEB_SERVER_USE
    gAppController->app_install(&server_app);
#endif
#if APP_IDEA_ANIM_USE
    gAppController->app_install(&idea_app);
#endif
#if APP_SETTING_USE
    gAppController->app_install(&settings_app);
#endif
#if APP_GAME_2048_USE
    gAppController->app_install(&game_2048_app);
#endif
#if APP_ANNIVERSARY_USE
    gAppController->app_install(&anniversary_app);
#endif
#if APP_HEARTBEAT_USE
    gAppController->app_install(&heartbeat_app, APP_TYPE_BACKGROUND);
#endif

    // 自启动APP
    gAppController->app_auto_start();

#if LV_USE_LOG
    lv_log_register_print_cb(minimjt_print);
#endif /*LV_USE_LOG*/
}

void loop()
{
    gAppController->main_process(); // 运行当前进程
}