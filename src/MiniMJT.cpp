#include "app/app_conf.h"
#include "common.h"
#include "driver/lv_port_indev.h"
#include "sys/app_controller.h"
#include <SPIFFS.h>

#if LV_USE_LOG
void LvglLog(const char *buf)
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

    g_appController = new AppController(); // APP控制器

    g_appController->Init();

    // 将APP"安装"到controller里
#if APP_WEATHER_USE
    g_appController->AppInstall(&weather_app);
#endif
#if APP_PICTURE_USE
    g_appController->AppInstall(&picture_app);
#endif
#if APP_MEDIA_PLAYER_USE
    g_appController->AppInstall(&media_app);
#endif
#if APP_SCREEN_SHARE_USE
    g_appController->AppInstall(&screen_share_app);
#endif
#if APP_FILE_MANAGER_USE
    g_appController->AppInstall(&file_manager_app);
#endif
#if APP_WEB_SERVER_USE
    g_appController->AppInstall(&server_app);
#endif
#if APP_IDEA_ANIM_USE
    g_appController->AppInstall(&idea_app);
#endif
#if APP_SETTING_USE
    g_appController->AppInstall(&settings_app);
#endif
#if APP_GAME_2048_USE
    g_appController->AppInstall(&game_2048_app);
#endif
#if APP_ANNIVERSARY_USE
    g_appController->AppInstall(&anniversary_app);
#endif
#if APP_HEARTBEAT_USE
    g_appController->AppInstall(&heartbeat_app, APP_TYPE_BACKGROUND);
#endif

    g_appController->ExitLoadingGui();

    // 自启动APP
    g_appController->AppAutoStart();

#if LV_USE_LOG
    lv_log_register_print_cb(LvglLog);
#endif /*LV_USE_LOG*/
}

void loop()
{
    g_appController->MainProcess(); // 运行当前进程
}