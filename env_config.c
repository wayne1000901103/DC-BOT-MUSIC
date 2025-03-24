#include "env_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define PATH_SEPARATOR '\\'
#else
#include <unistd.h>
#include <sys/stat.h>
#define PATH_SEPARATOR '/'
#endif

// 全局配置
static Config* global_config = NULL;

// 创建目录
static void create_directory(const char* path) {
#ifdef _WIN32
    char* path_copy = strdup(path);
    char* p = path_copy;
    while ((p = strchr(p, PATH_SEPARATOR)) != NULL) {
        *p = '\0';
        _mkdir(path_copy);
        *p = PATH_SEPARATOR;
        p++;
    }
    _mkdir(path_copy);
    free(path_copy);
#else
    char* path_copy = strdup(path);
    char* p = path_copy;
    while ((p = strchr(p, PATH_SEPARATOR)) != NULL) {
        *p = '\0';
        mkdir(path_copy, 0755);
        *p = PATH_SEPARATOR;
        p++;
    }
    mkdir(path_copy, 0755);
    free(path_copy);
#endif
}

// 检查文件是否存在
static bool file_exists(const char* path) {
#ifdef _WIN32
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
#else
    struct stat st;
    return stat(path, &st) == 0;
#endif
}

// 获取配置文件路径
static char* get_config_path(const char* env_file) {
    char* config_path;
#ifdef _WIN32
    char* appdata = getenv("APPDATA");
    if (appdata) {
        size_t len = strlen(appdata) + strlen("\\DiscordMusicBot\\") + strlen(env_file) + 1;
        config_path = (char*)malloc(len);
        snprintf(config_path, len, "%s\\DiscordMusicBot\\%s", appdata, env_file);
    } else {
        config_path = strdup(env_file);
    }
#else
    char* home = getenv("HOME");
    if (home) {
        size_t len;
#ifdef REPLIT
        len = strlen(home) + strlen("/.config/discord-music-bot/") + strlen(env_file) + 1;
        config_path = (char*)malloc(len);
        snprintf(config_path, len, "%s/.config/discord-music-bot/%s", home, env_file);
#else
        len = strlen(home) + strlen("/.config/discord-music-bot/") + strlen(env_file) + 1;
        config_path = (char*)malloc(len);
        snprintf(config_path, len, "%s/.config/discord-music-bot/%s", home, env_file);
#endif
    } else {
        config_path = strdup(env_file);
    }
#endif
    return config_path;
}

// 获取缓存目录路径
static char* get_cache_dir(void) {
    char* cache_dir;
#ifdef _WIN32
    char* localappdata = getenv("LOCALAPPDATA");
    if (localappdata) {
        size_t len = strlen(localappdata) + strlen("\\DiscordMusicBot\\cache") + 1;
        cache_dir = (char*)malloc(len);
        snprintf(cache_dir, len, "%s\\DiscordMusicBot\\cache", localappdata);
    } else {
        cache_dir = strdup("cache");
    }
#else
    char* home = getenv("HOME");
    if (home) {
        size_t len;
#ifdef REPLIT
        len = strlen(home) + strlen("/.cache/discord-music-bot") + 1;
        cache_dir = (char*)malloc(len);
        snprintf(cache_dir, len, "%s/.cache/discord-music-bot", home);
#else
        len = strlen(home) + strlen("/.cache/discord-music-bot") + 1;
        cache_dir = (char*)malloc(len);
        snprintf(cache_dir, len, "%s/.cache/discord-music-bot", home);
#endif
    } else {
        cache_dir = strdup("cache");
    }
#endif
    return cache_dir;
}

// 获取日志目录路径
static char* get_log_dir(void) {
    char* log_dir;
#ifdef _WIN32
    char* appdata = getenv("APPDATA");
    if (appdata) {
        size_t len = strlen(appdata) + strlen("\\DiscordMusicBot\\logs") + 1;
        log_dir = (char*)malloc(len);
        snprintf(log_dir, len, "%s\\DiscordMusicBot\\logs", appdata);
    } else {
        log_dir = strdup("logs");
    }
#else
    char* home = getenv("HOME");
    if (home) {
        size_t len;
#ifdef REPLIT
        len = strlen(home) + strlen("/.local/share/discord-music-bot/logs") + 1;
        log_dir = (char*)malloc(len);
        snprintf(log_dir, len, "%s/.local/share/discord-music-bot/logs", home);
#else
        len = strlen(home) + strlen("/.local/share/discord-music-bot/logs") + 1;
        log_dir = (char*)malloc(len);
        snprintf(log_dir, len, "%s/.local/share/discord-music-bot/logs", home);
#endif
    } else {
        log_dir = strdup("logs");
    }
#endif
    return log_dir;
}

// 解析内存大小字符串
static size_t parse_memory_size(const char* str) {
    size_t size = 0;
    char unit = 'B';
    sscanf(str, "%zu%c", &size, &unit);
    
    switch (toupper(unit)) {
        case 'K': size *= 1024; break;
        case 'M': size *= 1024 * 1024; break;
        case 'G': size *= 1024 * 1024 * 1024; break;
    }
    
    return size;
}

// 解析时间字符串
static int parse_time(const char* str) {
    int time = 0;
    char unit = 's';
    sscanf(str, "%d%c", &time, &unit);
    
    switch (toupper(unit)) {
        case 'M': time *= 60; break;
        case 'H': time *= 3600; break;
        case 'D': time *= 86400; break;
    }
    
    return time;
}

// 加载环境变量配置
Config* load_env_config(const char* env_file) {
    if (global_config) {
        return global_config;
    }
    
    global_config = (Config*)calloc(1, sizeof(Config));
    if (!global_config) return NULL;
    
    char* config_path = get_config_path(env_file);
    if (!config_path) {
        free(global_config);
        global_config = NULL;
        return NULL;
    }
    
    // 如果配置文件不存在，创建默认配置
    if (!file_exists(config_path)) {
        char* dir = strdup(config_path);
        char* last_sep = strrchr(dir, PATH_SEPARATOR);
        if (last_sep) {
            *last_sep = '\0';
            create_directory(dir);
        }
        free(dir);
        
        // 设置默认值
        global_config->audio_sample_rate = 44100;
        global_config->audio_channels = 2;
        global_config->audio_buffer_size = 4096;
        global_config->audio_queue_size = 100;
        global_config->max_threads = 4;
        global_config->max_concurrent_tasks = 10;
        global_config->cpu_threshold = 80.0f;
        global_config->memory_threshold = 80.0f;
        global_config->default_volume = 1.0f;
        global_config->max_memory_usage = 512 * 1024 * 1024; // 512MB
        global_config->max_cpu_usage = 80.0f;
        global_config->max_disk_usage = 1024 * 1024 * 1024; // 1GB
        global_config->cache_expiry = 24 * 3600; // 24h
        global_config->timeout = 30;
        global_config->api_rate_limit = 100;
        global_config->max_connections = 50;
        
        // 设置默认目录
        global_config->cache_dir = get_cache_dir();
        global_config->log_file = get_log_dir();
        
        // 保存默认配置
        save_env_config(config_path);
    }
    
    FILE* fp = fopen(config_path, "r");
    if (!fp) {
        free(config_path);
        free(global_config);
        global_config = NULL;
        return NULL;
    }
    
    char line[1024];
    char key[256], value[768];
    
    while (fgets(line, sizeof(line), fp)) {
        // 跳过注释和空行
        if (line[0] == '#' || line[0] == '\n') continue;
        
        // 解析键值对
        if (sscanf(line, "%[^=]=%[^\n]", key, value) == 2) {
            // 去除空格
            char* k = key;
            char* v = value;
            while (isspace(*k)) k++;
            while (isspace(*v)) v++;
            
            // 存储配置值
            if (strcmp(k, "DISCORD_TOKEN") == 0) {
                global_config->discord_token = strdup(v);
            }
            else if (strcmp(k, "TARGET_CHANNEL_ID") == 0) {
                global_config->target_channel_id = strdup(v);
            }
            else if (strcmp(k, "AUDIO_SAMPLE_RATE") == 0) {
                global_config->audio_sample_rate = atoi(v);
            }
            else if (strcmp(k, "AUDIO_CHANNELS") == 0) {
                global_config->audio_channels = atoi(v);
            }
            else if (strcmp(k, "AUDIO_BUFFER_SIZE") == 0) {
                global_config->audio_buffer_size = atoi(v);
            }
            else if (strcmp(k, "AUDIO_QUEUE_SIZE") == 0) {
                global_config->audio_queue_size = atoi(v);
            }
            else if (strcmp(k, "MAX_THREADS") == 0) {
                global_config->max_threads = atoi(v);
            }
            else if (strcmp(k, "MAX_CONCURRENT_TASKS") == 0) {
                global_config->max_concurrent_tasks = atoi(v);
            }
            else if (strcmp(k, "CPU_THRESHOLD") == 0) {
                global_config->cpu_threshold = atof(v);
            }
            else if (strcmp(k, "MEMORY_THRESHOLD") == 0) {
                global_config->memory_threshold = atof(v);
            }
            else if (strcmp(k, "DEFAULT_VOLUME") == 0) {
                global_config->default_volume = atof(v);
            }
            else if (strcmp(k, "DEFAULT_DISTORTION") == 0) {
                global_config->default_distortion = atof(v);
            }
            else if (strcmp(k, "DEFAULT_CHORUS_RATE") == 0) {
                global_config->default_chorus_rate = atof(v);
            }
            else if (strcmp(k, "DEFAULT_CHORUS_DEPTH") == 0) {
                global_config->default_chorus_depth = atof(v);
            }
            else if (strcmp(k, "DEFAULT_CHORUS_MIX") == 0) {
                global_config->default_chorus_mix = atof(v);
            }
            else if (strcmp(k, "DEFAULT_FLANGER_RATE") == 0) {
                global_config->default_flanger_rate = atof(v);
            }
            else if (strcmp(k, "DEFAULT_FLANGER_DEPTH") == 0) {
                global_config->default_flanger_depth = atof(v);
            }
            else if (strcmp(k, "DEFAULT_FLANGER_MIX") == 0) {
                global_config->default_flanger_mix = atof(v);
            }
            else if (strcmp(k, "DEFAULT_PHASER_RATE") == 0) {
                global_config->default_phaser_rate = atof(v);
            }
            else if (strcmp(k, "DEFAULT_PHASER_DEPTH") == 0) {
                global_config->default_phaser_depth = atof(v);
            }
            else if (strcmp(k, "DEFAULT_PHASER_MIX") == 0) {
                global_config->default_phaser_mix = atof(v);
            }
            else if (strcmp(k, "DEFAULT_WAH_FREQ") == 0) {
                global_config->default_wah_freq = atof(v);
            }
            else if (strcmp(k, "DEFAULT_WAH_Q") == 0) {
                global_config->default_wah_q = atof(v);
            }
            else if (strcmp(k, "DEFAULT_WAH_MIX") == 0) {
                global_config->default_wah_mix = atof(v);
            }
            else if (strcmp(k, "MAX_MEMORY_USAGE") == 0) {
                global_config->max_memory_usage = parse_memory_size(v);
            }
            else if (strcmp(k, "MAX_CPU_USAGE") == 0) {
                global_config->max_cpu_usage = atof(v);
            }
            else if (strcmp(k, "MAX_DISK_USAGE") == 0) {
                global_config->max_disk_usage = parse_memory_size(v);
            }
            else if (strcmp(k, "CACHE_DIR") == 0) {
                global_config->cache_dir = strdup(v);
            }
            else if (strcmp(k, "MAX_CACHE_SIZE") == 0) {
                global_config->max_cache_size = parse_memory_size(v);
            }
            else if (strcmp(k, "CACHE_EXPIRY") == 0) {
                global_config->cache_expiry = parse_time(v);
            }
            else if (strcmp(k, "LOG_LEVEL") == 0) {
                global_config->log_level = strdup(v);
            }
            else if (strcmp(k, "LOG_FILE") == 0) {
                global_config->log_file = strdup(v);
            }
            else if (strcmp(k, "MAX_LOG_SIZE") == 0) {
                global_config->max_log_size = parse_memory_size(v);
            }
            else if (strcmp(k, "MAX_LOG_FILES") == 0) {
                global_config->max_log_files = atoi(v);
            }
            else if (strcmp(k, "PROXY_ENABLED") == 0) {
                global_config->proxy_enabled = (strcasecmp(v, "true") == 0);
            }
            else if (strcmp(k, "PROXY_URL") == 0) {
                global_config->proxy_url = strdup(v);
            }
            else if (strcmp(k, "PROXY_USERNAME") == 0) {
                global_config->proxy_username = strdup(v);
            }
            else if (strcmp(k, "PROXY_PASSWORD") == 0) {
                global_config->proxy_password = strdup(v);
            }
            else if (strcmp(k, "TIMEOUT") == 0) {
                global_config->timeout = atoi(v);
            }
            else if (strcmp(k, "ENCRYPTION_KEY") == 0) {
                global_config->encryption_key = strdup(v);
            }
            else if (strcmp(k, "API_RATE_LIMIT") == 0) {
                global_config->api_rate_limit = atoi(v);
            }
            else if (strcmp(k, "MAX_CONNECTIONS") == 0) {
                global_config->max_connections = atoi(v);
            }
        }
    }
    
    fclose(fp);
    free(config_path);
    return global_config;
}

// 释放配置
void free_env_config(Config* config) {
    if (!config) return;
    
    free(config->discord_token);
    free(config->target_channel_id);
    free(config->cache_dir);
    free(config->log_level);
    free(config->log_file);
    free(config->proxy_url);
    free(config->proxy_username);
    free(config->proxy_password);
    free(config->encryption_key);
    
    free(config);
    global_config = NULL;
}

// 获取配置值
const char* get_env_value(const char* key) {
    if (!global_config) return NULL;
    
    if (strcmp(key, "DISCORD_TOKEN") == 0) return global_config->discord_token;
    if (strcmp(key, "TARGET_CHANNEL_ID") == 0) return global_config->target_channel_id;
    if (strcmp(key, "CACHE_DIR") == 0) return global_config->cache_dir;
    if (strcmp(key, "LOG_LEVEL") == 0) return global_config->log_level;
    if (strcmp(key, "LOG_FILE") == 0) return global_config->log_file;
    if (strcmp(key, "PROXY_URL") == 0) return global_config->proxy_url;
    if (strcmp(key, "PROXY_USERNAME") == 0) return global_config->proxy_username;
    if (strcmp(key, "PROXY_PASSWORD") == 0) return global_config->proxy_password;
    if (strcmp(key, "ENCRYPTION_KEY") == 0) return global_config->encryption_key;
    
    return NULL;
}

// 设置配置值
void set_env_value(const char* key, const char* value) {
    if (!global_config) return;
    
    if (strcmp(key, "DISCORD_TOKEN") == 0) {
        free(global_config->discord_token);
        global_config->discord_token = strdup(value);
    }
    else if (strcmp(key, "TARGET_CHANNEL_ID") == 0) {
        free(global_config->target_channel_id);
        global_config->target_channel_id = strdup(value);
    }
    else if (strcmp(key, "CACHE_DIR") == 0) {
        free(global_config->cache_dir);
        global_config->cache_dir = strdup(value);
    }
    else if (strcmp(key, "LOG_LEVEL") == 0) {
        free(global_config->log_level);
        global_config->log_level = strdup(value);
    }
    else if (strcmp(key, "LOG_FILE") == 0) {
        free(global_config->log_file);
        global_config->log_file = strdup(value);
    }
    else if (strcmp(key, "PROXY_URL") == 0) {
        free(global_config->proxy_url);
        global_config->proxy_url = strdup(value);
    }
    else if (strcmp(key, "PROXY_USERNAME") == 0) {
        free(global_config->proxy_username);
        global_config->proxy_username = strdup(value);
    }
    else if (strcmp(key, "PROXY_PASSWORD") == 0) {
        free(global_config->proxy_password);
        global_config->proxy_password = strdup(value);
    }
    else if (strcmp(key, "ENCRYPTION_KEY") == 0) {
        free(global_config->encryption_key);
        global_config->encryption_key = strdup(value);
    }
}

// 保存配置到文件
void save_env_config(const char* env_file) {
    if (!global_config) return;
    
    char* config_path = get_config_path(env_file);
    if (!config_path) return;
    
    // 确保目录存在
    char* dir = strdup(config_path);
    char* last_sep = strrchr(dir, PATH_SEPARATOR);
    if (last_sep) {
        *last_sep = '\0';
        create_directory(dir);
    }
    free(dir);
    
    FILE* fp = fopen(config_path, "w");
    if (!fp) {
        free(config_path);
        return;
    }
    
    fprintf(fp, "# Discord 配置\n");
    fprintf(fp, "DISCORD_TOKEN=%s\n", global_config->discord_token);
    fprintf(fp, "TARGET_CHANNEL_ID=%s\n", global_config->target_channel_id);
    
    fprintf(fp, "\n# 音频处理配置\n");
    fprintf(fp, "AUDIO_SAMPLE_RATE=%d\n", global_config->audio_sample_rate);
    fprintf(fp, "AUDIO_CHANNELS=%d\n", global_config->audio_channels);
    fprintf(fp, "AUDIO_BUFFER_SIZE=%d\n", global_config->audio_buffer_size);
    fprintf(fp, "AUDIO_QUEUE_SIZE=%d\n", global_config->audio_queue_size);
    
    fprintf(fp, "\n# 后台管理配置\n");
    fprintf(fp, "MAX_THREADS=%d\n", global_config->max_threads);
    fprintf(fp, "MAX_CONCURRENT_TASKS=%d\n", global_config->max_concurrent_tasks);
    fprintf(fp, "CPU_THRESHOLD=%.1f\n", global_config->cpu_threshold);
    fprintf(fp, "MEMORY_THRESHOLD=%.1f\n", global_config->memory_threshold);
    
    fprintf(fp, "\n# 音频效果默认参数\n");
    fprintf(fp, "DEFAULT_VOLUME=%.1f\n", global_config->default_volume);
    fprintf(fp, "DEFAULT_DISTORTION=%.1f\n", global_config->default_distortion);
    fprintf(fp, "DEFAULT_CHORUS_RATE=%.1f\n", global_config->default_chorus_rate);
    fprintf(fp, "DEFAULT_CHORUS_DEPTH=%.1f\n", global_config->default_chorus_depth);
    fprintf(fp, "DEFAULT_CHORUS_MIX=%.1f\n", global_config->default_chorus_mix);
    fprintf(fp, "DEFAULT_FLANGER_RATE=%.1f\n", global_config->default_flanger_rate);
    fprintf(fp, "DEFAULT_FLANGER_DEPTH=%.1f\n", global_config->default_flanger_depth);
    fprintf(fp, "DEFAULT_FLANGER_MIX=%.1f\n", global_config->default_flanger_mix);
    fprintf(fp, "DEFAULT_PHASER_RATE=%.1f\n", global_config->default_phaser_rate);
    fprintf(fp, "DEFAULT_PHASER_DEPTH=%.1f\n", global_config->default_phaser_depth);
    fprintf(fp, "DEFAULT_PHASER_MIX=%.1f\n", global_config->default_phaser_mix);
    fprintf(fp, "DEFAULT_WAH_FREQ=%.1f\n", global_config->default_wah_freq);
    fprintf(fp, "DEFAULT_WAH_Q=%.1f\n", global_config->default_wah_q);
    fprintf(fp, "DEFAULT_WAH_MIX=%.1f\n", global_config->default_wah_mix);
    
    fprintf(fp, "\n# 系统资源限制\n");
    fprintf(fp, "MAX_MEMORY_USAGE=%zuMB\n", global_config->max_memory_usage / (1024 * 1024));
    fprintf(fp, "MAX_CPU_USAGE=%.1f%%\n", global_config->max_cpu_usage);
    fprintf(fp, "MAX_DISK_USAGE=%zuGB\n", global_config->max_disk_usage / (1024 * 1024 * 1024));
    
    fprintf(fp, "\n# 缓存配置\n");
    fprintf(fp, "CACHE_DIR=%s\n", global_config->cache_dir);
    fprintf(fp, "MAX_CACHE_SIZE=%zuMB\n", global_config->max_cache_size / (1024 * 1024));
    fprintf(fp, "CACHE_EXPIRY=%dh\n", global_config->cache_expiry / 3600);
    
    fprintf(fp, "\n# 日志配置\n");
    fprintf(fp, "LOG_LEVEL=%s\n", global_config->log_level);
    fprintf(fp, "LOG_FILE=%s\n", global_config->log_file);
    fprintf(fp, "MAX_LOG_SIZE=%zuMB\n", global_config->max_log_size / (1024 * 1024));
    fprintf(fp, "MAX_LOG_FILES=%d\n", global_config->max_log_files);
    
    fprintf(fp, "\n# 网络配置\n");
    fprintf(fp, "PROXY_ENABLED=%s\n", global_config->proxy_enabled ? "true" : "false");
    fprintf(fp, "PROXY_URL=%s\n", global_config->proxy_url);
    fprintf(fp, "PROXY_USERNAME=%s\n", global_config->proxy_username);
    fprintf(fp, "PROXY_PASSWORD=%s\n", global_config->proxy_password);
    fprintf(fp, "TIMEOUT=%d\n", global_config->timeout);
    
    fprintf(fp, "\n# 安全配置\n");
    fprintf(fp, "ENCRYPTION_KEY=%s\n", global_config->encryption_key);
    fprintf(fp, "API_RATE_LIMIT=%d\n", global_config->api_rate_limit);
    fprintf(fp, "MAX_CONNECTIONS=%d\n", global_config->max_connections);
    
    fclose(fp);
    free(config_path);
} 