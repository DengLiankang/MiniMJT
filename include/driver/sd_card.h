#ifndef _SD_CARD_H_
#define _SD_CARD_H_

#include "driver/minimjt_fs.h"
#include "SD.h"

#define DIR_FILE_NUM 10
#define DIR_FILE_NAME_MAX_LEN 20
#define FILENAME_MAX_LEN 100

extern int photo_file_num;
extern char file_name_list[DIR_FILE_NUM][DIR_FILE_NAME_MAX_LEN];

enum FILE_TYPE : unsigned char {
    FILE_TYPE_UNKNOW = 0,
    FILE_TYPE_FILE,
    FILE_TYPE_FOLDER,
};

struct File_Info {
    char *file_name;
    FILE_TYPE file_type;
    File_Info *front_node; // 上一个节点
    File_Info *next_node;  // 下一个节点
};

void release_file_info(File_Info *info);

void join_path(char *dst_path, const char *pre_path, const char *rear_path);

// static const char *get_file_basename(const char *path);

class SdCard : public MiniMjtFs
{
public:
    SdCard();

    ~SdCard();

    void Init(void);

    File open(const String &path, const char *mode);

    File_Info *ListDir(const char *dirName);
};

#endif
