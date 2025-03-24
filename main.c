#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "audio_effects.h"
#include "music_recommender.h"
#include "lyrics_sync.h"
#include "discord_api.h"

// 音乐播放器结构体
typedef struct {
    char* current_track;
    char* current_url;
    float volume;
    int loop;
    int shuffle;
    TrackHistory* history;
    AudioProcessor* audio_processor;
    AudioVisualizer* visualizer;
    LyricSync* lyrics;
    int is_playing;
    pthread_mutex_t lock;
} MusicPlayer;

// 全局变量
MusicPlayer* player = NULL;
char* DISCORD_TOKEN = NULL;
char* TARGET_CHANNEL_ID = "1352656026821726340";

// 初始化音乐播放器
MusicPlayer* init_music_player() {
    MusicPlayer* p = (MusicPlayer*)malloc(sizeof(MusicPlayer));
    p->current_track = NULL;
    p->current_url = NULL;
    p->volume = 1.0f;
    p->loop = 0;
    p->shuffle = 0;
    p->history = init_track_history();
    p->audio_processor = init_audio_processor();
    p->visualizer = init_visualizer(1024, 2048);
    p->lyrics = init_lyric_sync();
    p->is_playing = 0;
    pthread_mutex_init(&p->lock, NULL);
    return p;
}

// 释放音乐播放器
void free_music_player(MusicPlayer* p) {
    if (p) {
        free(p->current_track);
        free(p->current_url);
        free_track_history(p->history);
        free_audio_processor(p->audio_processor);
        free_visualizer(p->visualizer);
        free_lyric_sync(p->lyrics);
        pthread_mutex_destroy(&p->lock);
        free(p);
    }
}

// 处理音频数据
void* process_audio(void* arg) {
    MusicPlayer* p = (MusicPlayer*)arg;
    float* buffer = (float*)malloc(4096 * sizeof(float));
    
    while (p->is_playing) {
        // 读取音频数据
        if (read_audio_data(buffer, 4096) > 0) {
            // 应用音频效果
            apply_effects(buffer, 4096, p->audio_processor);
            
            // 更新可视化
            update_visualization(buffer, 4096, p->visualizer);
            
            // 更新歌词同步
            update_time(p->lyrics, get_current_time());
            
            // 播放处理后的音频
            play_audio_data(buffer, 4096);
        }
    }
    
    free(buffer);
    return NULL;
}

// 处理Discord命令
void handle_discord_command(const char* command) {
    char cmd[256];
    char arg[1024];
    sscanf(command, "%s %[^\n]", cmd, arg);
    
    if (strcmp(cmd, "!play") == 0) {
        // 播放音乐
        char* url = search_music(arg);
        if (url) {
            pthread_mutex_lock(&player->lock);
            if (player->current_track) free(player->current_track);
            if (player->current_url) free(player->current_url);
            player->current_track = strdup(arg);
            player->current_url = strdup(url);
            player->is_playing = 1;
            pthread_mutex_unlock(&player->lock);
            
            // 启动音频处理线程
            pthread_t audio_thread;
            pthread_create(&audio_thread, NULL, process_audio, player);
            pthread_detach(audio_thread);
        }
    }
    else if (strcmp(cmd, "!effects") == 0) {
        // 设置音频效果
        char effect[64];
        float params[3];
        sscanf(arg, "%s %f %f %f", effect, &params[0], &params[1], &params[2]);
        
        pthread_mutex_lock(&player->lock);
        if (strcmp(effect, "distortion") == 0) {
            set_distortion(player->audio_processor, params[0]);
        }
        else if (strcmp(effect, "chorus") == 0) {
            set_chorus(player->audio_processor, params[0], params[1], params[2]);
        }
        else if (strcmp(effect, "flanger") == 0) {
            set_flanger(player->audio_processor, params[0], params[1], params[2]);
        }
        else if (strcmp(effect, "phaser") == 0) {
            set_phaser(player->audio_processor, params[0], params[1], params[2]);
        }
        else if (strcmp(effect, "wah") == 0) {
            set_wah(player->audio_processor, params[0], params[1], params[2]);
        }
        pthread_mutex_unlock(&player->lock);
    }
    else if (strcmp(cmd, "!visualize") == 0) {
        // 生成音频可视化
        pthread_mutex_lock(&player->lock);
        generate_visualization(player->visualizer, "visualization.png");
        pthread_mutex_unlock(&player->lock);
        
        // 发送图片到Discord
        send_discord_file("visualization.png");
    }
    else if (strcmp(cmd, "!lyrics") == 0) {
        // 搜索并显示歌词
        char* lrc_content = search_lyrics(arg);
        if (lrc_content) {
            pthread_mutex_lock(&player->lock);
            parse_lrc(player->lyrics, lrc_content);
            const char* current = get_current_lyric(player->lyrics);
            pthread_mutex_unlock(&player->lock);
            
            if (current) {
                send_discord_message(current);
            }
        }
    }
    else if (strcmp(cmd, "!recommend") == 0) {
        // 获取音乐推荐
        pthread_mutex_lock(&player->lock);
        int* recommendations = get_recommendations(player->history, 5);
        pthread_mutex_unlock(&player->lock);
        
        char msg[1024] = "推荐歌曲：\n";
        for (int i = 0; i < 5; i++) {
            char track[256];
            sprintf(track, "- %s\n", get_track_title(player->history, recommendations[i]));
            strcat(msg, track);
        }
        send_discord_message(msg);
        
        free(recommendations);
    }
}

// 主函数
int main(int argc, char* argv[]) {
    // 初始化
    curl_global_init(CURL_GLOBAL_DEFAULT);
    json_object* config = json_object_from_file("config.json");
    DISCORD_TOKEN = strdup(json_object_get_string(json_object_object_get(config, "token")));
    
    // 初始化音乐播放器
    player = init_music_player();
    
    // 初始化Discord API
    init_discord_api(DISCORD_TOKEN);
    
    // 主循环
    while (1) {
        char* command = receive_discord_command();
        if (command) {
            handle_discord_command(command);
            free(command);
        }
        usleep(100000);  // 100ms延迟
    }
    
    // 清理
    free_discord_api();
    free_music_player(player);
    free(DISCORD_TOKEN);
    json_object_put(config);
    curl_global_cleanup();
    
    return 0;
} 