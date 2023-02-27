#include "Arduino.h"
#include "common.h"
#include "sys/app_controller.h"
#include "sys/interface.h"

#define SYS_CONFIG_PATH         "/sys.cfg"
#define MPU_CONFIG_PATH         "/mpu.cfg"
#define MAX_CFG_INFO_LENGTH     64

static char gCfgInfo[MAX_CFG_INFO_LENGTH];

void AppController::ReadConfig(SysUtilConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    uint16_t size = gFlashCfg.readFile(SYS_CONFIG_PATH, (uint8_t *)gCfgInfo);
    gCfgInfo[size] = 0;
    if (size == 0) {
        // 默认值
        cfg->power_mode = 0;           // 功耗模式（0为节能模式 1为性能模式）
        cfg->backLight = 80;           // 屏幕亮度（1-100）
        cfg->rotation = 4;             // 屏幕旋转方向
        cfg->auto_calibration_mpu = 1; // 是否自动校准陀螺仪 0关闭自动校准 1打开自动校准
        cfg->mpu_order = 0;            // 操作方向
        cfg->auto_start_app = "None";  // 无指定开机自启APP
        this->WriteConfig(cfg);
    } else {
        // 解析数据
        char *param[12] = {0};
        analyseParam(gCfgInfo, 12, param);
        cfg->ssid_0 = param[0];
        cfg->password_0 = param[1];
        cfg->ssid_1 = param[2];
        cfg->password_1 = param[3];
        cfg->ssid_2 = param[4];
        cfg->password_2 = param[5];
        cfg->power_mode = atol(param[6]);
        cfg->backLight = atol(param[7]);
        cfg->rotation = atol(param[8]);
        cfg->auto_calibration_mpu = atol(param[9]);
        cfg->mpu_order = atol(param[10]);
        cfg->auto_start_app = param[11]; // 开机自启APP的name
    }
}

void AppController::WriteConfig(SysUtilConfig *cfg)
{
    char tmp[25];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    w_data = w_data + cfg->ssid_0 + "\n";
    w_data = w_data + cfg->password_0 + "\n";
    w_data = w_data + cfg->ssid_1 + "\n";
    w_data = w_data + cfg->password_1 + "\n";
    w_data = w_data + cfg->ssid_2 + "\n";
    w_data = w_data + cfg->password_2 + "\n";
    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->power_mode);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->backLight);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->rotation);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->auto_calibration_mpu);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->mpu_order);
    w_data += tmp;

    w_data = w_data + cfg->auto_start_app + "\n";

    gFlashCfg.writeFile(SYS_CONFIG_PATH, w_data.c_str());

    // 立即生效相关配置
    screen.setBackLight(cfg->backLight / 100.0);
    tft->setRotation(cfg->rotation);
    mpu.setOrder(cfg->mpu_order);
}

void AppController::ReadConfig(SysMpuConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    uint16_t size = gFlashCfg.readFile(MPU_CONFIG_PATH, (uint8_t *)gCfgInfo);
    gCfgInfo[size] = 0;
    if (size == 0) {
        // 默认值
        cfg->x_gyro_offset = 0;
        cfg->y_gyro_offset = 0;
        cfg->z_gyro_offset = 0;
        cfg->x_accel_offset = 0;
        cfg->y_accel_offset = 0;
        cfg->z_accel_offset = 0;

        this->WriteConfig(cfg);
    } else {
        // 解析数据
        char *param[6] = {0};
        analyseParam(gCfgInfo, 6, param);
        cfg->x_gyro_offset = atol(param[0]);
        cfg->y_gyro_offset = atol(param[1]);
        cfg->z_gyro_offset = atol(param[2]);
        cfg->x_accel_offset = atol(param[3]);
        cfg->y_accel_offset = atol(param[4]);
        cfg->z_accel_offset = atol(param[5]);
    }
}

void AppController::WriteConfig(SysMpuConfig *cfg)
{
    char tmp[25];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->x_gyro_offset);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->y_gyro_offset);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->z_gyro_offset);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->x_accel_offset);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->y_accel_offset);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->z_accel_offset);
    w_data += tmp;
    gFlashCfg.writeFile(MPU_CONFIG_PATH, w_data.c_str());
}

void AppController::deal_config(APP_MESSAGE_TYPE type, const char *key, char *value)
{
    switch (type) {

        case APP_MESSAGE_GET_PARAM: {
            if (!strcmp(key, "ssid_0")) {
                snprintf(value, 32, "%s", m_sysCfg.ssid_0.c_str());
            } else if (!strcmp(key, "password_0")) {
                snprintf(value, 32, "%s", m_sysCfg.password_0.c_str());
            } else if (!strcmp(key, "ssid_1")) {
                snprintf(value, 32, "%s", m_sysCfg.ssid_1.c_str());
            } else if (!strcmp(key, "password_1")) {
                snprintf(value, 32, "%s", m_sysCfg.password_1.c_str());
            }
            if (!strcmp(key, "ssid_2")) {
                snprintf(value, 32, "%s", m_sysCfg.ssid_2.c_str());
            } else if (!strcmp(key, "password_2")) {
                snprintf(value, 32, "%s", m_sysCfg.password_2.c_str());
            } else if (!strcmp(key, "power_mode")) {
                snprintf(value, 32, "%u", m_sysCfg.power_mode);
            } else if (!strcmp(key, "backLight")) {
                snprintf(value, 32, "%u", m_sysCfg.backLight);
            } else if (!strcmp(key, "rotation")) {
                snprintf(value, 32, "%u", m_sysCfg.rotation);
            } else if (!strcmp(key, "auto_calibration_mpu")) {
                snprintf(value, 32, "%u", m_sysCfg.auto_calibration_mpu);
            } else if (!strcmp(key, "mpu_order")) {
                snprintf(value, 32, "%u", m_sysCfg.mpu_order);
            } else if (!strcmp(key, "auto_start_app")) {
                snprintf(value, 32, "%s", m_sysCfg.auto_start_app.c_str());
            }
        } break;
        case APP_MESSAGE_SET_PARAM: {
            if (!strcmp(key, "ssid_0")) {
                m_sysCfg.ssid_0 = value;
            } else if (!strcmp(key, "password_0")) {
                m_sysCfg.password_0 = value;
            } else if (!strcmp(key, "ssid_1")) {
                m_sysCfg.ssid_1 = value;
            } else if (!strcmp(key, "password_1")) {
                m_sysCfg.password_1 = value;
            } else if (!strcmp(key, "ssid_2")) {
                m_sysCfg.ssid_2 = value;
            } else if (!strcmp(key, "password_2")) {
                m_sysCfg.password_2 = value;
            } else if (!strcmp(key, "power_mode")) {
                m_sysCfg.power_mode = atol(value);
            } else if (!strcmp(key, "backLight")) {
                m_sysCfg.backLight = atol(value);
            } else if (!strcmp(key, "rotation")) {
                m_sysCfg.rotation = atol(value);
            } else if (!strcmp(key, "auto_calibration_mpu")) {
                m_sysCfg.auto_calibration_mpu = atol(value);
                if (0 == m_sysCfg.auto_calibration_mpu) {
                    this->WriteConfig(&this->m_imuCfg);
                }
            } else if (!strcmp(key, "mpu_order")) {
                m_sysCfg.mpu_order = atol(value);
            } else if (!strcmp(key, "auto_start_app")) {
                m_sysCfg.auto_start_app = value;
            }
        } break;
        case APP_MESSAGE_READ_CFG: {
            ReadConfig(&m_sysCfg);
            // ReadConfig(&m_imuCfg);
        } break;
        case APP_MESSAGE_WRITE_CFG: {
            WriteConfig(&m_sysCfg);
            // WriteConfig(&m_imuCfg);  // 在取消自动校准的时候已经写过一次了
        } break;
        default:
            break;
    }
}