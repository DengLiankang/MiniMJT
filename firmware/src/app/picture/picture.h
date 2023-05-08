#ifndef APP_PICTURE_H
#define APP_PICTURE_H

#include "sys/interface.h"
#include "common.h"
#include "picture_gui.h"
// Include the jpeg decoder library
#include <TJpg_Decoder.h>

#define IMAGE_PATH "/image"

struct PICTURE_APP_CONFIG {
    unsigned long autoSwitchInterval; // 自动播放下一张的时间间隔 ms
};

class PictureApp {
public:
    unsigned long pic_perMillis; // 图片上一回更新的时间

    File_Info *image_file;      // movie文件夹下的文件指针头
    File_Info *pfile;           // 指向当前播放的文件节点
    int image_pos_increate = 1; // 文件的遍历方向
    bool refreshFlag = false;   // 是否更新
    bool tftSwapStatus;

public:
    PictureApp();
    ~PictureApp();
};

extern APP_OBJ PICTURE_APP;

#endif