#include "discord_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <libwebsockets.h>

// 全局变量
static char* DISCORD_TOKEN = NULL;
static CURL* curl = NULL;
static struct lws_context* ws_context = NULL;
static struct lws* ws_client = NULL;
static pthread_mutex_t ws_lock = PTHREAD_MUTEX_INITIALIZER;
static char* last_command = NULL;

// WebSocket 回调函数
static int ws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                      void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_RECEIVE:
            // 处理接收到的消息
            pthread_mutex_lock(&ws_lock);
            if (last_command) free(last_command);
            last_command = strdup((char*)in);
            pthread_mutex_unlock(&ws_lock);
            break;
            
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            // 发送身份验证消息
            json_object* identify = json_object_new_object();
            json_object_object_add(identify, "op", json_object_new_int(2));
            
            json_object* d = json_object_new_object();
            json_object_object_add(d, "token", json_object_new_string(DISCORD_TOKEN));
            json_object_object_add(d, "intents", json_object_new_int(513)); // 消息内容和语音状态
            json_object_object_add(d, "properties", json_object_new_object());
            
            json_object_object_add(identify, "d", d);
            
            const char* json_str = json_object_to_json_string(identify);
            lws_write(wsi, (unsigned char*)json_str, strlen(json_str), LWS_WRITE_TEXT);
            
            json_object_put(identify);
            break;
    }
    return 0;
}

// WebSocket 协议
static struct lws_protocols protocols[] = {
    {
        "discord-protocol",
        ws_callback,
        0,
        4096,
    },
    { NULL, NULL, 0, 0 }
};

// 初始化 Discord API
void init_discord_api(const char* token) {
    DISCORD_TOKEN = strdup(token);
    
    // 初始化 CURL
    curl = curl_easy_init();
    
    // 初始化 WebSocket
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    
    ws_context = lws_create_context(&info);
    
    // 创建 WebSocket 连接
    struct lws_client_connect_info conn_info;
    memset(&conn_info, 0, sizeof(conn_info));
    conn_info.context = ws_context;
    conn_info.address = "gateway.discord.gg";
    conn_info.port = 443;
    conn_info.path = "/?v=10&encoding=json";
    conn_info.host = conn_info.address;
    conn_info.origin = conn_info.address;
    conn_info.protocol = protocols[0].name;
    conn_info.ssl_connection = LCCSCF_USE_SSL | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
    
    ws_client = lws_client_connect_via_info(&conn_info);
}

// 释放 Discord API
void free_discord_api(void) {
    if (ws_client) {
        lws_callback_on_writable(ws_client);
        lws_client_destroy_context(ws_context);
    }
    
    if (curl) {
        curl_easy_cleanup(curl);
    }
    
    if (DISCORD_TOKEN) {
        free(DISCORD_TOKEN);
    }
    
    if (last_command) {
        free(last_command);
    }
    
    pthread_mutex_destroy(&ws_lock);
}

// 发送消息到 Discord
int send_discord_message(const char* content) {
    if (!curl || !DISCORD_TOKEN) return -1;
    
    char url[256];
    sprintf(url, "%s/channels/%s/messages", DISCORD_API_BASE, TARGET_CHANNEL_ID);
    
    json_object* message = json_object_new_object();
    json_object_object_add(message, "content", json_object_new_string(content));
    
    const char* json_str = json_object_to_json_string(message);
    
    struct curl_slist* headers = NULL;
    char auth_header[256];
    sprintf(auth_header, "Authorization: Bot %s", DISCORD_TOKEN);
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    json_object_put(message);
    
    return (res == CURLE_OK) ? 0 : -1;
}

// 发送文件到 Discord
int send_discord_file(const char* filepath) {
    if (!curl || !DISCORD_TOKEN) return -1;
    
    char url[256];
    sprintf(url, "%s/channels/%s/messages", DISCORD_API_BASE, TARGET_CHANNEL_ID);
    
    struct curl_httppost* formpost = NULL;
    struct curl_httppost* lastptr = NULL;
    
    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "file",
                 CURLFORM_FILE, filepath,
                 CURLFORM_END);
    
    struct curl_slist* headers = NULL;
    char auth_header[256];
    sprintf(auth_header, "Authorization: Bot %s", DISCORD_TOKEN);
    headers = curl_slist_append(headers, auth_header);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_formfree(formpost);
    curl_slist_free_all(headers);
    
    return (res == CURLE_OK) ? 0 : -1;
}

// 接收 Discord 命令
char* receive_discord_command(void) {
    pthread_mutex_lock(&ws_lock);
    char* command = last_command;
    last_command = NULL;
    pthread_mutex_unlock(&ws_lock);
    
    return command;
}

// 搜索音乐
char* search_music(const char* query) {
    // 这里实现音乐搜索逻辑
    // 可以使用 YouTube Data API 或其他音乐平台的 API
    return NULL;
}

// 搜索歌词
char* search_lyrics(const char* query) {
    // 这里实现歌词搜索逻辑
    // 可以使用歌词网站的 API
    return NULL;
} 