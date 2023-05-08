#ifndef WEB_SETTING_H
#define WEB_SETTING_H

#include "sys/app_controller.h"
#include <WString.h>

void init_page_header(void);
void init_page_footer(void);
void HomePage(void);

void File_Download(void);
void File_Upload(void);
void File_Delete(void);
void delete_result(void);
void handleFileUpload(void);

void sys_setting(void);
void weather_setting(void);
void picture_setting(void);
void media_setting(void);
void screen_setting(void);
void heartbeat_setting(void);
void anniversary_setting(void);

void saveSysConf(void);
void saveWeatherConf(void);
void savePictureConf(void);
void saveMediaConf(void);
void saveScreenConf(void);
void saveHeartbeatConf(void);
void saveAnniversaryConf(void);

void sd_file_download(const String &filename);
void SelectInput(String heading, String command, String arg_calling_name);
void ReportSDNotPresent(void);
void ReportFileNotPresent(const String &target);
void ReportCouldNotCreateFile(const String &target);

#endif