#ifndef _APP_CONF_H_
#define _APP_CONF_H_

/**
 * 将某项APP对应的宏定义设置为1，可以将对应APP的
 * 代码编译进固件，设置为0则不编译进固件。便于腾
 * 出额外的flash空间给其他用户自定义app。
 * 注: 当某个app不使能时，对应的web配置项也不会
 * 出现在配置网页上。
 */

// 在开发新的app时，可以先把其他所有APP关闭，这样烧录速度也比较快

// 纪念日APP
#define APP_ANNIVERSARY_USE 0
#if APP_ANNIVERSARY_USE
#include "app/anniversary/anniversary.h"
#endif

// 文件管理器APP
#define APP_FILE_MANAGER_USE 0
#if APP_FILE_MANAGER_USE
#include "app/file_manager/file_manager.h"
#endif

// 2048游戏APP
#define APP_GAME_2048_USE 0
#if APP_GAME_2048_USE
#include "app/game_2048/game_2048.h"
#endif

// 心跳APP
#define APP_HEARTBEAT_USE 0
#if APP_HEARTBEAT_USE
#include "app/heartbeat/heartbeat.h"
#endif

// 特效动画APP
#define APP_IDEA_ANIM_USE 0
#if APP_IDEA_ANIM_USE
#include "app/idea_anim/idea.h"
#endif

// 视频播放器APP
#define APP_MEDIA_PLAYER_USE 0
#if APP_MEDIA_PLAYER_USE
#include "app/media_player/media_player.h"
#endif

// 相册APP
#define APP_PICTURE_USE 1
#if APP_PICTURE_USE
#include "app/picture/picture.h"
#endif

// 网页配置服务APP
// 此项用于配置小电视，为必选项，此处仅为了说明有这个app，此宏无作用
#define APP_WEB_SERVER_USE 1
#if APP_WEB_SERVER_USE
#include "app/server/server.h"
#endif

// 设置APP
#define APP_SETTING_USE 0
#if APP_SETTING_USE
#include "app/settings/settings.h"
#endif

// 天气APP
#define APP_WEATHER_USE 1
#if APP_WEATHER_USE
#include "app/weather/weather.h"
#endif

#endif