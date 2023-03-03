#include "FS.h"
#include "FFat.h"
#include <Arduino.h>

class FlashFs
{
public:
    FlashFs();

    ~FlashFs();

    void Init(void);

    void ListDir(const char *dirname, uint8_t levels);

    uint16_t ReadFile(const char *path, uint8_t *info);

    void WriteFile(const char *path, const char *message);

    void AppendFile(const char *path, const char *message);

    void RenameFile(const char *src, const char *dst);

    void DeleteFile(const char *path);
};
