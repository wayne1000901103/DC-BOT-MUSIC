#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 歌词行结构体
typedef struct {
    char* text;
    double start_time;
    double end_time;
    int line_number;
} LyricLine;

// 歌词同步器结构体
typedef struct {
    LyricLine* lines;
    int num_lines;
    int current_line;
    double current_time;
    double offset;
    int is_playing;
} LyricSync;

// 初始化歌词同步器
LyricSync* init_lyric_sync() {
    LyricSync* sync = (LyricSync*)malloc(sizeof(LyricSync));
    sync->lines = NULL;
    sync->num_lines = 0;
    sync->current_line = -1;
    sync->current_time = 0.0;
    sync->offset = 0.0;
    sync->is_playing = 0;
    return sync;
}

// 解析LRC格式歌词
void parse_lrc(LyricSync* sync, const char* lrc_content) {
    char* content = strdup(lrc_content);
    char* line = strtok(content, "\n");
    int capacity = 100;
    sync->lines = (LyricLine*)malloc(capacity * sizeof(LyricLine));
    
    while (line) {
        if (strlen(line) < 10) {  // 跳过空行或无效行
            line = strtok(NULL, "\n");
            continue;
        }
        
        // 解析时间戳 [mm:ss.xx]
        int minutes, seconds, hundredths;
        if (sscanf(line, "[%d:%d.%d]", &minutes, &seconds, &hundredths) == 3) {
            double time = minutes * 60.0 + seconds + hundredths / 100.0;
            
            // 跳过时间戳，获取歌词文本
            char* text = strchr(line, ']') + 1;
            while (*text == ' ') text++;  // 跳过空格
            
            if (sync->num_lines >= capacity) {
                capacity *= 2;
                sync->lines = (LyricLine*)realloc(sync->lines, capacity * sizeof(LyricLine));
            }
            
            sync->lines[sync->num_lines].text = strdup(text);
            sync->lines[sync->num_lines].start_time = time;
            sync->lines[sync->num_lines].line_number = sync->num_lines;
            sync->num_lines++;
        }
        
        line = strtok(NULL, "\n");
    }
    
    // 设置每行的结束时间
    for (int i = 0; i < sync->num_lines - 1; i++) {
        sync->lines[i].end_time = sync->lines[i + 1].start_time;
    }
    if (sync->num_lines > 0) {
        sync->lines[sync->num_lines - 1].end_time = sync->lines[sync->num_lines - 1].start_time + 5.0;
    }
    
    free(content);
}

// 更新当前时间
void update_time(LyricSync* sync, double time) {
    sync->current_time = time + sync->offset;
    
    // 查找当前应该显示的歌词行
    for (int i = 0; i < sync->num_lines; i++) {
        if (sync->current_time >= sync->lines[i].start_time && 
            sync->current_time < sync->lines[i].end_time) {
            if (i != sync->current_line) {
                sync->current_line = i;
                // 这里可以添加回调函数来显示歌词
                printf("当前歌词: %s\n", sync->lines[i].text);
            }
            break;
        }
    }
}

// 设置时间偏移
void set_offset(LyricSync* sync, double offset) {
    sync->offset = offset;
}

// 获取当前歌词
const char* get_current_lyric(LyricSync* sync) {
    if (sync->current_line >= 0 && sync->current_line < sync->num_lines) {
        return sync->lines[sync->current_line].text;
    }
    return NULL;
}

// 获取下一行歌词
const char* get_next_lyric(LyricSync* sync) {
    if (sync->current_line + 1 < sync->num_lines) {
        return sync->lines[sync->current_line + 1].text;
    }
    return NULL;
}

// 获取上一行歌词
const char* get_previous_lyric(LyricSync* sync) {
    if (sync->current_line > 0) {
        return sync->lines[sync->current_line - 1].text;
    }
    return NULL;
}

// 获取当前行号
int get_current_line_number(LyricSync* sync) {
    return sync->current_line;
}

// 获取总行数
int get_total_lines(LyricSync* sync) {
    return sync->num_lines;
}

// 释放歌词同步器
void free_lyric_sync(LyricSync* sync) {
    for (int i = 0; i < sync->num_lines; i++) {
        free(sync->lines[i].text);
    }
    free(sync->lines);
    free(sync);
} 