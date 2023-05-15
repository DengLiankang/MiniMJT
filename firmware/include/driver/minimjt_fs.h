#ifndef _MINIMJT_FS_
#define _MINIMJT_FS_

#include "FS.h"

#define FS_IS_NULL(fs)                                                                                                 \
    if (NULL == fs) {                                                                                                  \
        Serial.println(F("FS not Mounted"));                                                                           \
        return -1;                                                                                                     \
    }

class MiniMjtFs
{
public:
    fs::FS *m_fs;

public:
    MiniMjtFs();

    ~MiniMjtFs();

    virtual int8_t Init(void) = 0;

    int8_t ListDir(const char *dirName, uint8_t levels);

    int16_t ReadFile(const char *path, uint8_t *info);

    int8_t WriteFile(const char *path, const char *message);

    int8_t AppendFile(const char *path, const char *message);

    int8_t RenameFile(const char *src, const char *dst);

    int8_t DeleteFile(const char *path);

    bool FileExists(const char *path);
};

#endif