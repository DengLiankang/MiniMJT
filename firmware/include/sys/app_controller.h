#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include "Arduino.h"
#include "common.h"
#include "interface.h"
#include <vector>

#define APP_MAX_NUM 10             // 最大的可运行的APP数量
#define WIFI_LIFE_CYCLE 60000      // wifi的生命周期（60s）
#define MQTT_ALIVE_CYCLE 1000      // mqtt重连周期
#define EVENT_LIST_MAX_LENGTH 10   // 消息队列的容量

// 系统状态
// STATE_SYS_LOADING 开机加载阶段
// STATE_APP_MENU app菜单界面
// STATE_APP_RUNNING 前台运行某个app
enum MJT_SYS_STATE {
    STATE_SYS_LOADING,
    STATE_APP_MENU,
    STATE_APP_RUNNING,
};

class AppController
{
public:
    AppController(const char *name = MJT_APP_CTRL);
    ~AppController();

    // 初始化
    void Init(void);

    int AppAutoStart();

    // 退出loading界面，进入app menu阶段
    void ExitLoadingGui(void);

    // 将APP注册到app_controller中
    int AppInstall(APP_OBJ *app, APP_TYPE appType = APP_TYPE_REALTIME);

    void MainProcess(void);
    void AppExit(void); // 提供给app退出的系统调用
    // 消息发送
    int SendRequestEvent(const char *from, const char *to, APP_MESSAGE_TYPE type, void *message, void *ext_info);
private:
    // 获取系统当前状态
    MJT_SYS_STATE GetSystemState(void);

    // 设置系统当前状态
    void SetSystemState(MJT_SYS_STATE state);

    void ReadConfig(SysUtilConfig *cfg);
    void WriteConfig(SysUtilConfig *cfg);
    APP_OBJ *GetAppByName(const char *name);
    int GetAppIndexByName(const char *name);
    int AppIsLegal(const APP_OBJ *appObj);
    void WifiRequestDeal(APP_MESSAGE_TYPE type);
    void UpdateWifiStatus(void);
    void GetParam(const char *key, char *value);
    void SetParam(const char *key, const char *value);
    void RequestProcess(APP_MESSAGE_TYPE type, void *data, void *extData);


private:
    const char *m_name; // app控制器的名字
    std::vector<APP_OBJ *> m_appList;     // 预留APP_MAX_NUM个APP注册位
    WIFI_STATUS m_wifiStatus;            // 表示是wifi状态
    unsigned long m_preWifiReqMillis; // 保存上一回请求的时间戳
    uint8_t m_wifiSsidItem;
    unsigned int m_appNum;
    int m_currentAppItem;     // 当前运行的APP下标
    const char *m_requestFrom;
    MJT_SYS_STATE m_appCtrlState;
    TimerHandle_t m_appCtrlTimer; // 事件处理定时器
    ImuAction *m_imuActionData;             // 存放mpu6050数据

    IMU m_imu;
    Network m_network; // 网络连接
    Display m_screen;    // 屏幕对象

public:
    SysUtilConfig m_sysCfg;
};

extern AppController *g_appController;

#endif