#include "driver/minimjt_fs.h"
#include "SPIFFS.h"
#include <Arduino.h>

class FlashFs : public MiniMjtFs
{
public:
    FlashFs();

    ~FlashFs();

    int8_t Init(void);
};
