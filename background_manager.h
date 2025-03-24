#ifndef BACKGROUND_MANAGER_H
#define BACKGROUND_MANAGER_H

#include <pthread.h>
#include <stdbool.h>

// 任务类型枚举
typedef enum {
    TASK_AUDIO_PROCESSING,
    TASK_MUSIC_DOWNLOAD,
    TASK_LYRICS_FETCH,
    TASK_RECOMMENDATION,
    TASK_VISUALIZATION
} TaskType;

// 任务结构体
typedef struct {
    TaskType type;
    void* data;
    void (*callback)(void*);
    int priority;
} Task;

// 线程池结构体
typedef struct {
    pthread_t* threads;
    Task* task_queue;
    int queue_size;
    int queue_capacity;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_cond;
    bool is_running;
    int num_threads;
} ThreadPool;

// 后台管理器结构体
typedef struct {
    ThreadPool* thread_pool;
    pthread_mutex_t resource_lock;
    int active_tasks;
    int max_concurrent_tasks;
    float cpu_threshold;
    float memory_threshold;
} BackgroundManager;

// 初始化后台管理器
BackgroundManager* init_background_manager(int num_threads, int max_tasks);

// 释放后台管理器
void free_background_manager(BackgroundManager* manager);

// 添加任务到队列
int add_task(BackgroundManager* manager, TaskType type, void* data, void (*callback)(void*), int priority);

// 获取系统资源使用情况
void get_system_metrics(float* cpu_usage, float* memory_usage);

// 动态调整线程池大小
void adjust_thread_pool(BackgroundManager* manager);

// 暂停任务处理
void pause_processing(BackgroundManager* manager);

// 恢复任务处理
void resume_processing(BackgroundManager* manager);

// 获取当前活动任务数
int get_active_tasks(BackgroundManager* manager);

// 设置资源使用阈值
void set_resource_thresholds(BackgroundManager* manager, float cpu_threshold, float memory_threshold);

#endif // BACKGROUND_MANAGER_H 