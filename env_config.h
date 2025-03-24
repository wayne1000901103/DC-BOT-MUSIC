#ifndef ENV_CONFIG_H
#define ENV_CONFIG_H

#include <stdbool.h>

// 配置结构体
typedef struct {
    // Discord配置
    char* discord_token;
    char* target_channel_id;
    
    // 音频处理配置
    int audio_sample_rate;
    int audio_channels;
    int audio_buffer_size;
    int audio_queue_size;
    
    // 后台管理配置
    int max_threads;
    int max_concurrent_tasks;
    float cpu_threshold;
    float memory_threshold;
    
    // 音频效果默认参数
    float default_volume;
    float default_distortion;
    float default_chorus_rate;
    float default_chorus_depth;
    float default_chorus_mix;
    float default_flanger_rate;
    float default_flanger_depth;
    float default_flanger_mix;
    float default_phaser_rate;
    float default_phaser_depth;
    float default_phaser_mix;
    float default_wah_freq;
    float default_wah_q;
    float default_wah_mix;
    
    // 系统资源限制
    size_t max_memory_usage;
    float max_cpu_usage;
    size_t max_disk_usage;
    
    // 缓存配置
    char* cache_dir;
    size_t max_cache_size;
    int cache_expiry;
    
    // 日志配置
    char* log_level;
    char* log_file;
    size_t max_log_size;
    int max_log_files;
    
    // 网络配置
    bool proxy_enabled;
    char* proxy_url;
    char* proxy_username;
    char* proxy_password;
    int timeout;
    
    // 安全配置
    char* encryption_key;
    int api_rate_limit;
    int max_connections;
} Config;

// 加载环境变量配置
Config* load_env_config(const char* env_file);

// 释放配置
void free_env_config(Config* config);

// 获取配置值
const char* get_env_value(const char* key);

// 设置配置值
void set_env_value(const char* key, const char* value);

// 保存配置到文件
void save_env_config(const char* env_file);

#endif // ENV_CONFIG_H 