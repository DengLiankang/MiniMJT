#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include "Arduino.h"
#include "common.h"
#include "driver/imu.h"
#include "interface.h"
#include <list>

#define CTRL_NAME "MJT_AppCtrl"
#define APP_MAX_NUM 20             // 最大的可运行的APP数量
#define WIFI_LIFE_CYCLE 60000      // wifi的生命周期（60s）
#define MQTT_ALIVE_CYCLE 1000      // mqtt重连周期
#define EVENT_LIST_MAX_LENGTH 10   // 消息队列的容量
#define APP_CONTROLLER_NAME_LEN 16 // app控制器的名字长度

// struct EVENT_OBJ
// {
//     const APP_OBJ *from;   // 发送请求服务的APP
//     const APP_OBJ *to;     // 接受请求服务的APP
//     APP_MESSAGE_TYPE type; // app的消息类型
//     unsigned int id;       // 发送请求服务的id
//     void *message;         // 附带数据，可以为任何数据类型
// };

// 系统状态
// STATE_SYS_LOADING 开机加载阶段
// STATE_APP_MENU app菜单界面
// STATE_APP_RUNNING 前台运行某个app
enum MJT_SYS_STATE {
    STATE_SYS_LOADING,
    STATE_APP_MENU,
    STATE_APP_RUNNING,
};

struct EVENT_OBJ {
    const APP_OBJ *from;       // 发送请求服务的APP
    APP_MESSAGE_TYPE type;     // app的事件类型
    void *info;                // 请求携带的信息
    uint8_t retryMaxNum;       // 重试次数
    uint8_t retryCount;        // 重试计数
    unsigned long nextRunTime; // 下次运行的时间戳
};

class AppController
{
public:
    AppController(const char *name = CTRL_NAME);
    ~AppController();

    // 初始化driver gui
    void Init(void);

    // 获取系统当前状态
    MJT_SYS_STATE GetSystemState(void);

    // 设置系统当前状态
    void SetSystemState(MJT_SYS_STATE state);

    int AppAutoStart();

    // 将APP注册到app_controller中
    int AppInstall(APP_OBJ *app, APP_TYPE appType = APP_TYPE_REALTIME);

    // 退出loading界面，进入app menu阶段
    void ExitLoadingGui(void);

    // 将APP的后台任务从任务队列中移除(自能通过APP退出的时候，移除自身的后台任务)
    int remove_backgroud_task(void);
    void MainProcess();
    void AppExit(void); // 提供给app退出的系统调用
    // 消息发送
    int send_to(const char *from, const char *to, APP_MESSAGE_TYPE type, void *message, void *ext_info);
    void deal_config(APP_MESSAGE_TYPE type, const char *key, char *value);

    // 事件处理
    int req_event_deal(void);

    // wifi事件的处理
    bool wifi_event(APP_MESSAGE_TYPE type);

    void ReadConfig(SysUtilConfig *cfg);
    void WriteConfig(SysUtilConfig *cfg);
    void ReadConfig(SysMpuConfig *cfg);
    void WriteConfig(SysMpuConfig *cfg);

private:
    APP_OBJ *GetAppByName(const char *name);
    int GetAppIndexByName(const char *name);
    int AppIsLegal(const APP_OBJ *appObj);

private:
    char m_name[APP_CONTROLLER_NAME_LEN]; // app控制器的名字
    APP_OBJ *m_appList[APP_MAX_NUM];     // 预留APP_MAX_NUM个APP注册位
    APP_TYPE m_appTypeList[APP_MAX_NUM];  // 对应APP的运行类型
    // std::list<const APP_OBJ *> app_list; // APP注册位(为了C语言可移植，放弃使用链表)
    std::list<EVENT_OBJ> eventList;   // 用来储存事件
    boolean m_wifi_status;            // 表示是wifi状态 true开启 false关闭
    unsigned long m_preWifiReqMillis; // 保存上一回请求的时间戳
    unsigned int m_appNum;
    int m_currentAppItem;     // 当前运行的APP下标
    MJT_SYS_STATE m_appCtrlState;
    TimerHandle_t m_appCtrlTimer; // 事件处理定时器

public:
    SysUtilConfig m_sysCfg;
    SysMpuConfig m_imuCfg;
};

extern AppController *g_appController;

#endif