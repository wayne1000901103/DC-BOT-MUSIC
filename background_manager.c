#include "background_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <time.h>

// 线程工作函数
static void* thread_worker(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    Task task;
    
    while (pool->is_running) {
        pthread_mutex_lock(&pool->queue_lock);
        while (pool->queue_size == 0 && pool->is_running) {
            pthread_cond_wait(&pool->queue_cond, &pool->queue_lock);
        }
        
        if (!pool->is_running) {
            pthread_mutex_unlock(&pool->queue_lock);
            break;
        }
        
        // 获取优先级最高的任务
        int highest_priority = -1;
        int highest_index = -1;
        for (int i = 0; i < pool->queue_size; i++) {
            if (pool->task_queue[i].priority > highest_priority) {
                highest_priority = pool->task_queue[i].priority;
                highest_index = i;
            }
        }
        
        if (highest_index != -1) {
            task = pool->task_queue[highest_index];
            // 移除任务
            for (int i = highest_index; i < pool->queue_size - 1; i++) {
                pool->task_queue[i] = pool->task_queue[i + 1];
            }
            pool->queue_size--;
        }
        pthread_mutex_unlock(&pool->queue_lock);
        
        if (highest_index != -1) {
            // 执行任务
            if (task.callback) {
                task.callback(task.data);
            }
        }
    }
    
    return NULL;
}

// 初始化后台管理器
BackgroundManager* init_background_manager(int num_threads, int max_tasks) {
    BackgroundManager* manager = (BackgroundManager*)malloc(sizeof(BackgroundManager));
    
    // 初始化线程池
    manager->thread_pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    manager->thread_pool->num_threads = num_threads;
    manager->thread_pool->threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    manager->thread_pool->task_queue = (Task*)malloc(max_tasks * sizeof(Task));
    manager->thread_pool->queue_capacity = max_tasks;
    manager->thread_pool->queue_size = 0;
    manager->thread_pool->is_running = true;
    
    pthread_mutex_init(&manager->thread_pool->queue_lock, NULL);
    pthread_cond_init(&manager->thread_pool->queue_cond, NULL);
    
    // 初始化其他参数
    pthread_mutex_init(&manager->resource_lock, NULL);
    manager->active_tasks = 0;
    manager->max_concurrent_tasks = max_tasks;
    manager->cpu_threshold = 80.0f;  // 默认CPU使用率阈值
    manager->memory_threshold = 80.0f;  // 默认内存使用率阈值
    
    // 启动工作线程
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&manager->thread_pool->threads[i], NULL, thread_worker, manager->thread_pool);
    }
    
    return manager;
}

// 释放后台管理器
void free_background_manager(BackgroundManager* manager) {
    if (!manager) return;
    
    // 停止所有线程
    manager->thread_pool->is_running = false;
    pthread_cond_broadcast(&manager->thread_pool->queue_cond);
    
    // 等待所有线程结束
    for (int i = 0; i < manager->thread_pool->num_threads; i++) {
        pthread_join(manager->thread_pool->threads[i], NULL);
    }
    
    // 释放资源
    free(manager->thread_pool->threads);
    free(manager->thread_pool->task_queue);
    free(manager->thread_pool);
    
    pthread_mutex_destroy(&manager->resource_lock);
    pthread_mutex_destroy(&manager->thread_pool->queue_lock);
    pthread_cond_destroy(&manager->thread_pool->queue_cond);
    
    free(manager);
}

// 添加任务到队列
int add_task(BackgroundManager* manager, TaskType type, void* data, void (*callback)(void*), int priority) {
    if (!manager || !manager->thread_pool) return -1;
    
    pthread_mutex_lock(&manager->thread_pool->queue_lock);
    
    if (manager->thread_pool->queue_size >= manager->thread_pool->queue_capacity) {
        pthread_mutex_unlock(&manager->thread_pool->queue_lock);
        return -1;
    }
    
    Task task = {
        .type = type,
        .data = data,
        .callback = callback,
        .priority = priority
    };
    
    manager->thread_pool->task_queue[manager->thread_pool->queue_size++] = task;
    pthread_cond_signal(&manager->thread_pool->queue_cond);
    
    pthread_mutex_unlock(&manager->thread_pool->queue_lock);
    return 0;
}

// 获取系统资源使用情况
void get_system_metrics(float* cpu_usage, float* memory_usage) {
    // 获取CPU使用率
    static struct timespec last_cpu_time;
    static struct timespec current_cpu_time;
    static unsigned long long last_total_cpu = 0;
    static unsigned long long last_idle_cpu = 0;
    
    FILE* fp = fopen("/proc/stat", "r");
    if (fp) {
        unsigned long long total_cpu = 0;
        unsigned long long idle_cpu = 0;
        char line[256];
        
        if (fgets(line, sizeof(line), fp)) {
            sscanf(line, "cpu %llu %llu %llu %llu", &idle_cpu, &total_cpu, &total_cpu, &total_cpu);
            total_cpu += idle_cpu;
            
            clock_gettime(CLOCK_MONOTONIC, &current_cpu_time);
            double time_diff = (current_cpu_time.tv_sec - last_cpu_time.tv_sec) +
                             (current_cpu_time.tv_nsec - last_cpu_time.tv_nsec) / 1e9;
            
            if (time_diff >= 1.0) {
                *cpu_usage = 100.0f * (1.0f - (float)(idle_cpu - last_idle_cpu) / (float)(total_cpu - last_total_cpu));
                last_total_cpu = total_cpu;
                last_idle_cpu = idle_cpu;
                last_cpu_time = current_cpu_time;
            }
        }
        fclose(fp);
    }
    
    // 获取内存使用率
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        unsigned long total_ram = si.totalram;
        unsigned long free_ram = si.freeram;
        *memory_usage = 100.0f * (1.0f - (float)free_ram / (float)total_ram);
    }
}

// 动态调整线程池大小
void adjust_thread_pool(BackgroundManager* manager) {
    if (!manager) return;
    
    float cpu_usage, memory_usage;
    get_system_metrics(&cpu_usage, &memory_usage);
    
    pthread_mutex_lock(&manager->resource_lock);
    
    // 根据资源使用情况调整线程数
    if (cpu_usage > manager->cpu_threshold || memory_usage > manager->memory_threshold) {
        // 减少线程数
        if (manager->thread_pool->num_threads > 2) {
            manager->thread_pool->num_threads--;
            // 停止一个线程
            pthread_cancel(manager->thread_pool->threads[manager->thread_pool->num_threads]);
        }
    } else if (cpu_usage < manager->cpu_threshold * 0.7f && 
               memory_usage < manager->memory_threshold * 0.7f) {
        // 增加线程数
        if (manager->thread_pool->num_threads < manager->max_concurrent_tasks) {
            pthread_create(&manager->thread_pool->threads[manager->thread_pool->num_threads],
                          NULL, thread_worker, manager->thread_pool);
            manager->thread_pool->num_threads++;
        }
    }
    
    pthread_mutex_unlock(&manager->resource_lock);
}

// 暂停任务处理
void pause_processing(BackgroundManager* manager) {
    if (!manager) return;
    
    pthread_mutex_lock(&manager->thread_pool->queue_lock);
    manager->thread_pool->is_running = false;
    pthread_mutex_unlock(&manager->thread_pool->queue_lock);
}

// 恢复任务处理
void resume_processing(BackgroundManager* manager) {
    if (!manager) return;
    
    pthread_mutex_lock(&manager->thread_pool->queue_lock);
    manager->thread_pool->is_running = true;
    pthread_cond_broadcast(&manager->thread_pool->queue_cond);
    pthread_mutex_unlock(&manager->thread_pool->queue_lock);
}

// 获取当前活动任务数
int get_active_tasks(BackgroundManager* manager) {
    if (!manager) return 0;
    
    int count;
    pthread_mutex_lock(&manager->resource_lock);
    count = manager->active_tasks;
    pthread_mutex_unlock(&manager->resource_lock);
    
    return count;
}

// 设置资源使用阈值
void set_resource_thresholds(BackgroundManager* manager, float cpu_threshold, float memory_threshold) {
    if (!manager) return;
    
    pthread_mutex_lock(&manager->resource_lock);
    manager->cpu_threshold = cpu_threshold;
    manager->memory_threshold = memory_threshold;
    pthread_mutex_unlock(&manager->resource_lock);
} 