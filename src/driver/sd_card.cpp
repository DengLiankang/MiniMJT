#include "driver/sd_card.h"
#include "common.h"
#include <string.h>

int photo_file_num = 0;
char file_name_list[DIR_FILE_NUM][DIR_FILE_NAME_MAX_LEN];

void release_file_info(File_Info *info)
{
    File_Info *cur_node = NULL; // 记录当前节点
    if (NULL == info) {
        return;
    }
    for (cur_node = info->next_node; NULL != cur_node;) {
        // 判断是不是循环一圈回来了
        if (info->next_node == cur_node) {
            break;
        }
        File_Info *tmp = cur_node; // 保存准备删除的节点
        cur_node = cur_node->next_node;
        free(tmp);
    }
    free(info);
}

void join_path(char *dst_path, const char *pre_path, const char *rear_path)
{
    while (*pre_path != 0) {
        *dst_path = *pre_path;
        ++dst_path;
        ++pre_path;
    }
    if (*(pre_path - 1) != '/') {
        *dst_path = '/';
        ++dst_path;
    }

    if (*rear_path == '/') {
        ++rear_path;
    }
    while (*rear_path != 0) {
        *dst_path = *rear_path;
        ++dst_path;
        ++rear_path;
    }
    *dst_path = 0;
}

/*
 * get file basename
 */
static const char *get_file_basename(const char *path)
{
    // 获取最后一个'/'所在的下标
    const char *ret = path;
    for (const char *cur = path; *cur != 0; ++cur) {
        if (*cur == '/') {
            ret = cur + 1;
        }
    }
    return ret;
}

SdCard::SdCard() {}

SdCard::~SdCard() {}

void SdCard::Init()
{
    SPIClass *sdSpi = new SPIClass(HSPI);          // another SPI
    sdSpi->begin(SD_SCK, SD_MISO, SD_MOSI, SD_SS); // Replace default HSPI pins
    if (!SD.begin(SD_SS, *sdSpi, 80000000))        // SD-Card SS pin is 15
    {
        Serial.println("Card Mount Failed");
        return;
    }
    m_fs = &SD;
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    Serial.printf("SD Card Size: %uMB\n", (uint32_t)(SD.cardSize() >> 20));
}

File_Info *SdCard::ListDir(const char *dirName)
{
    Serial.printf("Listing directory: %s\n", dirName);

    File root = m_fs->open(dirName);
    if (!root) {
        Serial.println("Failed to open directory");
        return NULL;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return NULL;
    }

    int dir_len = strlen(dirName) + 1;

    // 头节点的创建（头节点用来记录此文件夹）
    File_Info *head_file = (File_Info *)malloc(sizeof(File_Info));
    head_file->file_type = FILE_TYPE_FOLDER;
    head_file->file_name = (char *)malloc(dir_len);
    // 将文件夹名赋值给头节点（当作这个节点的文件名）
    strncpy(head_file->file_name, dirName, dir_len - 1);
    head_file->file_name[dir_len - 1] = 0;
    head_file->front_node = NULL;
    head_file->next_node = NULL;

    File_Info *file_node = head_file;

    File file = root.openNextFile();
    while (file) {
        // if (levels)
        // {
        //     listDir(file.name(), levels - 1);
        // }
        const char *fn = get_file_basename(file.name());
        // 字符数组长度为实际字符串长度+1
        int filename_len = strlen(fn);
        if (filename_len > FILENAME_MAX_LEN - 10) {
            Serial.println("Filename is too long.");
        }

        // 创建新节点
        file_node->next_node = (File_Info *)malloc(sizeof(File_Info));
        // 让下一个节点指向当前节点
        // （此时第一个节点的front_next会指向head节点，等遍历结束再调一下）
        file_node->next_node->front_node = file_node;
        // file_node指针移向节点
        file_node = file_node->next_node;

        // 船家创建新节点的文件名
        file_node->file_name = (char *)malloc(filename_len);
        strncpy(file_node->file_name, fn, filename_len); //
        file_node->file_name[filename_len] = 0;          //
        // 下一个节点赋空
        file_node->next_node = NULL;

        char tmp_file_name[FILENAME_MAX_LEN] = {0};
        // sprintf(tmp_file_name, "%s/%s", dirName, file_node->file_name);
        join_path(tmp_file_name, dirName, file_node->file_name);
        if (file.isDirectory()) {
            file_node->file_type = FILE_TYPE_FOLDER;
            // 类型为文件夹
            Serial.print("  DIR : ");
            Serial.println(tmp_file_name);
        } else {
            file_node->file_type = FILE_TYPE_FILE;
            // 类型为文件
            Serial.print("  FILE: ");
            Serial.print(tmp_file_name);
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }

        file = root.openNextFile();
    }

    if (NULL != head_file->next_node) {
        // 将最后一个节点的next_node指针指向 head_file->next_node
        file_node->next_node = head_file->next_node;
        // 调整第一个数据节点的front_node指针（非head节点）
        head_file->next_node->front_node = file_node;
    }
    return head_file;
}

// void SdCard::createDir(const char *path)
// {
//     FS_IS_NULL()

//     Serial.printf("Creating Dir: %s\n", path);
//     if (g_tfVfs->mkdir(path)) {
//         Serial.println("Dir created");
//     } else {
//         Serial.println("mkdir failed");
//     }
// }

// void SdCard::removeDir(const char *path)
// {
//     FS_IS_NULL()

//     Serial.printf("Removing Dir: %s\n", path);
//     if (g_tfVfs->rmdir(path)) {
//         Serial.println("Dir removed");
//     } else {
//         Serial.println("rmdir failed");
//     }
// }

// void SdCard::ReadFile(const char *path)
// {
//     FS_IS_NULL()

//     Serial.printf("Reading file: %s\n", path);

//     File file = g_tfVfs->open(path);
//     if (!file) {
//         Serial.println("Failed to open file for reading");
//         return;
//     }

//     Serial.print("Read from file: ");
//     while (file.available()) {
//         Serial.write(file.read());
//     }
//     file.close();
// }

// String SdCard::ReadFileLine(const char *path, int num)
// {
//     FS_IS_NULL("")

//     Serial.printf("Reading file: %s line: %d\n", path, num);

//     File file = g_tfVfs->open(path);
//     if (!file) {
//         return ("Failed to open file for reading");
//     }

//     char *p = buf;
//     while (file.available()) {
//         char c = file.read();
//         if (c == '\n') {
//             num--;
//             if (num == 0) {
//                 *(p++) = '\0';
//                 String s(buf);
//                 s.trim();
//                 return s;
//             }
//         } else if (num == 1) {
//             *(p++) = c;
//         }
//     }
//     file.close();

//     return String("error parameter!");
// }

// void SdCard::WriteFile(const char *path, const char *info)
// {
//     FS_IS_NULL()

//     Serial.printf("Writing file: %s\n", path);

//     File file = g_tfVfs->open(path, FILE_WRITE);
//     if (!file) {
//         Serial.println("Failed to open file for writing");
//         return;
//     }
//     if (file.println(info)) {
//         Serial.println("Write succ");
//     } else {
//         Serial.println("Write failed");
//     }
//     file.close();
// }

File SdCard::open(const String &path, const char *mode)
{
    // FS_IS_NULL(RET)

    return m_fs->open(path, mode);
}

// void SdCard::appendFile(const char *path, const char *message)
// {
//     FS_IS_NULL()

//     Serial.printf("Appending to file: %s\n", path);

//     File file = g_tfVfs->open(path, FILE_APPEND);
//     if (!file) {
//         Serial.println("Failed to open file for appending");
//         return;
//     }
//     if (file.print(message)) {
//         Serial.println("Message appended");
//     } else {
//         Serial.println("Append failed");
//     }
//     file.close();
// }

// void SdCard::renameFile(const char *path1, const char *path2)
// {
//     FS_IS_NULL()

//     Serial.printf("Renaming file %s to %s\n", path1, path2);
//     if (g_tfVfs->rename(path1, path2)) {
//         Serial.println("File renamed");
//     } else {
//         Serial.println("Rename failed");
//     }
// }

// boolean SdCard::deleteFile(const char *path)
// {
//     FS_IS_NULL(false)

//     Serial.printf("Deleting file: %s\n", path);
//     if (g_tfVfs->remove(path)) {
//         Serial.println("File deleted");
//         return true;
//     } else {
//         Serial.println("Delete failed");
//     }
//     return false;
// }
