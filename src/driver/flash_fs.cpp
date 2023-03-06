#include "driver/flash_fs.h"

FlashFs::FlashFs() {}

FlashFs::~FlashFs() {}

void FlashFs::Init(void)
{
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    m_fs = &SPIFFS;
    Serial.printf("SPIFFS Mount Success\nTotal Space: %u\n", SPIFFS.totalBytes());
}
