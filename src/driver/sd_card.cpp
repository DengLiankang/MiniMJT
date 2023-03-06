#include "driver/sd_card.h"
#include "common.h"
#include <string.h>

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
