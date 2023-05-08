#include "picture.h"
#include "sys/app_controller.h"

#define PICTURE_APP_NAME "Picture"

// 相册的持久化配置
#define PICTURE_CONFIG_PATH "/picture.cfg"

static PICTURE_APP_CONFIG g_pictureAppCfg;
PictureApp *g_pictureApp = NULL;

static int8_t WriteConfigToFlash(PICTURE_APP_CONFIG *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->autoSwitchInterval);
    w_data += tmp;
    return g_flashFs.WriteFile(PICTURE_CONFIG_PATH, w_data.c_str());
}

static int16_t ReadConfigFromFlash(PICTURE_APP_CONFIG *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    int16_t size = g_flashFs.ReadFile(PICTURE_CONFIG_PATH, (uint8_t *)info);
    if (size >= 0) {
        info[size] = 0;
        // 解析数据
        char *param[1] = {0};
        ParseParam(info, 1, param);
        cfg->autoSwitchInterval = atol(param[0]);

    } else {
        cfg->autoSwitchInterval = 10000;
    }
    return 0;
}

static File_Info *get_next_file(File_Info *p_cur_file, int direction)
{
    // 得到 p_cur_file 的下一个 类型为FILE_TYPE_FILE 的文件（即下一个非文件夹文件）
    if (NULL == p_cur_file) {
        return NULL;
    }

    File_Info *pfile = direction == 1 ? p_cur_file->next_node : p_cur_file->front_node;
    while (pfile != p_cur_file) {
        if (FILE_TYPE_FILE == pfile->file_type) {
            break;
        }
        pfile = direction == 1 ? pfile->next_node : pfile->front_node;
    }
    return pfile;
}

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    // Stop further decoding as image is running off bottom of screen
    if (y >= tft->height())
        return 0;

    // This function will clip the image block rendering automatically at the TFT boundaries
    tft->pushImage(x, y, w, h, bitmap);

    // This might work instead if you adapt the sketch to use the Adafruit_GFX library
    // tft.drawRGBBitmap(x, y, bitmap, w, h);

    // Return 1 to decode next block
    return 1;
}

PictureApp::PictureApp() {
    pic_perMillis = 0;
    image_file = NULL;
    pfile = NULL;
    image_pos_increate = 1;
    // 保存系统的tft设置参数 用于退出时恢复设置
    tftSwapStatus = tft->getSwapBytes();
    tft->setSwapBytes(true); // We need to swap the colour bytes (endianess)
    // TODO 实现文件获取
    // run_data->image_file = g_tfCard.ListDir(IMAGE_PATH);
    if (NULL != image_file) {
        pfile = get_next_file(image_file->next_node, 1);
    }
    g_tfCard.ListDir("/image", 1);

    // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(1);
    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(tft_output);
}

PictureApp::~PictureApp() {}

static int PictureAppInit(AppController *sys)
{
    ReadConfigFromFlash(&g_pictureAppCfg);
    g_pictureApp = new PictureApp();
    photo_gui_init();
    display_photo("/image/pig.png", LV_SCR_LOAD_ANIM_FADE_ON);

    return 0;
}

static void PictureAppMainPorcess(AppController *sys, const ImuAction *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_FADE_ON;

    if (RETURN == act_info->active) {
        sys->AppExit();
        return;
    }

    if (TURN_RIGHT == act_info->active) {
        anim_type = LV_SCR_LOAD_ANIM_OVER_RIGHT;
        g_pictureApp->image_pos_increate = 1;
        g_pictureApp->refreshFlag = true;
    } else if (TURN_LEFT == act_info->active) {
        anim_type = LV_SCR_LOAD_ANIM_OVER_LEFT;
        g_pictureApp->image_pos_increate = -1;
        g_pictureApp->refreshFlag = true;
    }

    if (NULL == g_pictureApp->image_file) {
        sys->AppExit();
        return;
    }

    // 自动切换的时间检测
    if (0 != g_pictureApp->image_pos_increate && 0 != g_pictureAppCfg.autoSwitchInterval &&
        millis() - g_pictureApp->pic_perMillis >= g_pictureAppCfg.autoSwitchInterval) {
        g_pictureApp->refreshFlag = true;
    }

    if (true == g_pictureApp->refreshFlag) {
        if (NULL != g_pictureApp->image_file) {
            g_pictureApp->pfile = get_next_file(g_pictureApp->pfile, g_pictureApp->image_pos_increate);
        }
        char file_name[PIC_FILENAME_MAX_LEN] = {0};
        snprintf(file_name, PIC_FILENAME_MAX_LEN, "%s/%s", g_pictureApp->image_file->file_name,
                 g_pictureApp->pfile->file_name);
        // Draw the image, top left at 0,0
        Serial.print(F("Decode image: "));
        Serial.println(file_name);
        if (NULL != strstr(file_name, ".jpg") || NULL != strstr(file_name, ".JPG")) {
            // 直接解码jpg格式的图片
            TJpgDec.drawSdJpg(0, 0, file_name);
        } else if (NULL != strstr(file_name, ".bin") || NULL != strstr(file_name, ".BIN")) {
            // 使用LVGL的bin格式的图片
            display_photo(file_name, anim_type);
        }
        g_pictureApp->refreshFlag = false;
        // 重置更新的时间标记
        g_pictureApp->pic_perMillis = millis();
    }
    delay(300);
}

static int PictureAppExit(void *param)
{
    photo_gui_del();
    // 释放文件名链表
    release_file_info(g_pictureApp->image_file);
    // 恢复此前的驱动参数
    tft->setSwapBytes(g_pictureApp->tftSwapStatus);

    WriteConfigToFlash(&g_pictureAppCfg);

    // 释放运行数据
    if (NULL != g_pictureApp) {
        delete(g_pictureApp);
        g_pictureApp = NULL;
    }
    return 0;
}

static void PictureAppMessageHandle(const char *from, const char *to, APP_MESSAGE_TYPE type, void *message,
                                    void *ext_info)
{
    switch (type) {
        case APP_MESSAGE_GET_PARAM: {
            char *param_key = (char *)message;
            if (!strcmp(param_key, "switchInterval")) {
                snprintf((char *)ext_info, 32, "%lu", g_pictureAppCfg.autoSwitchInterval);
            } else {
                snprintf((char *)ext_info, 32, "%s", "NULL");
            }
        } break;
        case APP_MESSAGE_SET_PARAM: {
            char *param_key = (char *)message;
            char *param_val = (char *)ext_info;
            if (!strcmp(param_key, "switchInterval")) {
                g_pictureAppCfg.autoSwitchInterval = atol(param_val);
            }
        } break;
        case APP_MESSAGE_READ_CFG: {
            ReadConfigFromFlash(&g_pictureAppCfg);
        } break;
        case APP_MESSAGE_WRITE_CFG: {
            WriteConfigToFlash(&g_pictureAppCfg);
        } break;
        default:
            break;
    }
}

APP_OBJ PICTURE_APP = {PICTURE_APP_NAME, &PictureAppLogo,        "", PictureAppInit, PictureAppMainPorcess, NULL,
                       PictureAppExit,   PictureAppMessageHandle};