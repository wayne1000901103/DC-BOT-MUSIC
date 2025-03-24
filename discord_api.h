#ifndef DISCORD_API_H
#define DISCORD_API_H

#include <curl/curl.h>
#include <json-c/json.h>

// Discord API 配置
#define DISCORD_API_BASE "https://discord.com/api/v10"
#define DISCORD_WS_URL "wss://gateway.discord.gg/?v=10&encoding=json"

// 初始化 Discord API
void init_discord_api(const char* token);

// 释放 Discord API
void free_discord_api(void);

// 发送消息到 Discord
int send_discord_message(const char* content);

// 发送文件到 Discord
int send_discord_file(const char* filepath);

// 接收 Discord 命令
char* receive_discord_command(void);

// 搜索音乐
char* search_music(const char* query);

// 搜索歌词
char* search_lyrics(const char* query);

#endif // DISCORD_API_H 