#include "driver/minimjt_fs.h"
#include "SPIFFS.h"
#include <Arduino.h>

class FlashFs : public MiniMjtFs
{
public:
    FlashFs();

    ~FlashFs();

    void Init(void);
};
