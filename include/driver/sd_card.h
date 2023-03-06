#ifndef _SD_CARD_H_
#define _SD_CARD_H_

#include "driver/minimjt_fs.h"
#include "SD.h"

#define FILENAME_MAX_LEN 100

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

    File_Info *ListDir(const char *dirName);
};

#endif
