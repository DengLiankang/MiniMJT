#include "driver/minimjt_fs.h"

MiniMjtFs::MiniMjtFs() {}

MiniMjtFs::~MiniMjtFs() {}

int8_t MiniMjtFs::ListDir(const char *dirName, uint8_t levels)
{
    FS_IS_NULL(m_fs)
    Serial.printf("Listing directory: %s\r\n", dirName);

    File root = m_fs->open(dirName);
    if (!root) {
        Serial.println(F("- failed to open directory"));
        return -1;
    }
    if (!root.isDirectory()) {
        Serial.println(F("- not a directory"));
        return -1;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print(F("  DIR : "));
            Serial.println(file.name());
            if (levels) {
                ListDir(file.path(), levels - 1);
            }
        } else {
            Serial.print(F("  FILE: "));
            Serial.print(file.name());
            Serial.print(F("\tSIZE: "));
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
    return 0;
}

int16_t MiniMjtFs::ReadFile(const char *path, uint8_t *info)
{
    FS_IS_NULL(m_fs)
    Serial.printf("Reading file: %s\r\n", path);

    File file = m_fs->open(path);
    uint16_t retLen = -1;
    if (!file || file.isDirectory()) {
        Serial.println(F("- failed to open file for reading"));
        return retLen;
    }
    retLen = 0;
    while (file.available()) {
        retLen += file.read(info + retLen, 15);
    }
    file.close();
    return retLen;
}

int8_t MiniMjtFs::WriteFile(const char *path, const char *message)
{
    FS_IS_NULL(m_fs)
    int8_t ret = -1;
    Serial.printf("Writing file: %s\r\n", path);

    File file = m_fs->open(path, FILE_WRITE);
    if (!file) {
        Serial.println(F("- failed to open file for writing"));
        return ret;
    }
    if (file.print(message)) {
        Serial.println(F("- file written"));
        ret = 0;
    } else {
        Serial.println(F("- write failed"));
    }
    file.close();
    return ret;
}

int8_t MiniMjtFs::AppendFile(const char *path, const char *message)
{
    FS_IS_NULL(m_fs)
    int ret = -1;
    Serial.printf("Appending to file: %s\r\n", path);

    File file = m_fs->open(path, FILE_APPEND);
    if (!file) {
        Serial.println(F("- failed to open file for appending"));
        return ret;
    }
    if (file.print(message)) {
        Serial.println(F("- message appended"));
        ret = 0;
    } else {
        Serial.println(F("- append failed"));
    }
    file.close();
    return ret;
}

int8_t MiniMjtFs::RenameFile(const char *path1, const char *path2)
{
    FS_IS_NULL(m_fs)
    int8_t ret = 0;
    Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (m_fs->rename(path1, path2)) {
        Serial.println(F("- file renamed"));
    } else {
        Serial.println(F("- rename failed"));
        ret = -1;
    }
    return ret;
}

int8_t MiniMjtFs::DeleteFile(const char *path)
{
    FS_IS_NULL(m_fs)
    int8_t ret = 0;
    Serial.printf("Deleting file: %s\r\n", path);
    if (m_fs->remove(path)) {
        Serial.println(F("- file deleted"));
    } else {
        Serial.println(F("- delete failed"));
        ret = -1;
    }
    return ret;
}

bool MiniMjtFs::FileExists(const char *path)
{
    FS_IS_NULL(m_fs)
    return m_fs->exists(path);
}
