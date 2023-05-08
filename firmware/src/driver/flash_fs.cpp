#include "driver/flash_fs.h"

FlashFs::FlashFs() {}

FlashFs::~FlashFs() {}

int8_t FlashFs::Init(void)
{
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return -1;
    }
    m_fs = &SPIFFS;
    Serial.printf("SPIFFS Mount Success\nTotal Space: %u\n", SPIFFS.totalBytes());
    return 0;
}
