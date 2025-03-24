#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

// 音乐特征结构体
typedef struct {
    float tempo;
    float spectral_centroid;
    float spectral_rolloff;
    float* mfcc;
    int mfcc_size;
    float* chroma;
    int chroma_size;
    float* onset_strength;
    int onset_size;
    float* beat_times;
    int beat_size;
    float* key_features;
    int key_size;
} MusicFeatures;

// 音乐推荐系统结构体
typedef struct {
    MusicFeatures* features;
    int num_tracks;
    float* similarity_matrix;
    int* genre_indices;
    int num_genres;
} Recommender;

// 初始化音乐特征
MusicFeatures* init_music_features(int mfcc_size, int chroma_size, int onset_size, 
                                 int beat_size, int key_size) {
    MusicFeatures* features = (MusicFeatures*)malloc(sizeof(MusicFeatures));
    features->mfcc = (float*)malloc(mfcc_size * sizeof(float));
    features->chroma = (float*)malloc(chroma_size * sizeof(float));
    features->onset_strength = (float*)malloc(onset_size * sizeof(float));
    features->beat_times = (float*)malloc(beat_size * sizeof(float));
    features->key_features = (float*)malloc(key_size * sizeof(float));
    
    features->mfcc_size = mfcc_size;
    features->chroma_size = chroma_size;
    features->onset_size = onset_size;
    features->beat_size = beat_size;
    features->key_size = key_size;
    
    return features;
}

// 释放音乐特征
void free_music_features(MusicFeatures* features) {
    free(features->mfcc);
    free(features->chroma);
    free(features->onset_strength);
    free(features->beat_times);
    free(features->key_features);
    free(features);
}

// 计算欧氏距离
float euclidean_distance(float* vec1, float* vec2, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        float diff = vec1[i] - vec2[i];
        sum += diff * diff;
    }
    return sqrtf(sum);
}

// 计算余弦相似度
float cosine_similarity(float* vec1, float* vec2, int size) {
    float dot_product = 0.0f;
    float norm1 = 0.0f;
    float norm2 = 0.0f;
    
    for (int i = 0; i < size; i++) {
        dot_product += vec1[i] * vec2[i];
        norm1 += vec1[i] * vec1[i];
        norm2 += vec2[i] * vec2[i];
    }
    
    if (norm1 == 0.0f || norm2 == 0.0f) return 0.0f;
    return dot_product / (sqrtf(norm1) * sqrtf(norm2));
}

// 计算动态时间规整距离
float dtw_distance(float* seq1, float* seq2, int size1, int size2) {
    float** dtw = (float**)malloc((size1 + 1) * sizeof(float*));
    for (int i = 0; i <= size1; i++) {
        dtw[i] = (float*)malloc((size2 + 1) * sizeof(float));
        for (int j = 0; j <= size2; j++) {
            dtw[i][j] = FLT_MAX;
        }
    }
    
    dtw[0][0] = 0.0f;
    
    for (int i = 1; i <= size1; i++) {
        for (int j = 1; j <= size2; j++) {
            float cost = fabsf(seq1[i-1] - seq2[j-1]);
            dtw[i][j] = cost + fminf(fminf(dtw[i-1][j], dtw[i][j-1]), dtw[i-1][j-1]);
        }
    }
    
    float result = dtw[size1][size2];
    
    for (int i = 0; i <= size1; i++) {
        free(dtw[i]);
    }
    free(dtw);
    
    return result;
}

// 计算音乐特征之间的相似度
float compute_similarity(MusicFeatures* f1, MusicFeatures* f2) {
    // 计算各个特征的相似度
    float tempo_sim = 1.0f - fabsf(f1->tempo - f2->tempo) / fmaxf(f1->tempo, f2->tempo);
    float centroid_sim = 1.0f - fabsf(f1->spectral_centroid - f2->spectral_centroid) / 
                        fmaxf(f1->spectral_centroid, f2->spectral_centroid);
    float rolloff_sim = 1.0f - fabsf(f1->spectral_rolloff - f2->spectral_rolloff) / 
                       fmaxf(f1->spectral_rolloff, f2->spectral_rolloff);
    
    // 计算MFCC的DTW距离
    float mfcc_sim = 1.0f - dtw_distance(f1->mfcc, f2->mfcc, f1->mfcc_size, f2->mfcc_size) / 
                     fmaxf(f1->mfcc_size, f2->mfcc_size);
    
    // 计算色度特征的余弦相似度
    float chroma_sim = cosine_similarity(f1->chroma, f2->chroma, f1->chroma_size);
    
    // 计算节奏特征的相似度
    float onset_sim = cosine_similarity(f1->onset_strength, f2->onset_strength, f1->onset_size);
    
    // 计算调性特征的相似度
    float key_sim = cosine_similarity(f1->key_features, f2->key_features, f1->key_size);
    
    // 加权平均
    return 0.2f * tempo_sim + 0.1f * centroid_sim + 0.1f * rolloff_sim + 
           0.2f * mfcc_sim + 0.15f * chroma_sim + 0.15f * onset_sim + 0.1f * key_sim;
}

// 初始化推荐系统
Recommender* init_recommender(int num_tracks, int num_genres) {
    Recommender* rec = (Recommender*)malloc(sizeof(Recommender));
    rec->features = (MusicFeatures**)malloc(num_tracks * sizeof(MusicFeatures*));
    rec->num_tracks = num_tracks;
    rec->similarity_matrix = (float*)malloc(num_tracks * num_tracks * sizeof(float));
    rec->genre_indices = (int*)malloc(num_genres * sizeof(int));
    rec->num_genres = num_genres;
    return rec;
}

// 更新相似度矩阵
void update_similarity_matrix(Recommender* rec) {
    for (int i = 0; i < rec->num_tracks; i++) {
        for (int j = i + 1; j < rec->num_tracks; j++) {
            float similarity = compute_similarity(rec->features[i], rec->features[j]);
            rec->similarity_matrix[i * rec->num_tracks + j] = similarity;
            rec->similarity_matrix[j * rec->num_tracks + i] = similarity;
        }
        rec->similarity_matrix[i * rec->num_tracks + i] = 1.0f;
    }
}

// 获取推荐
int* get_recommendations(Recommender* rec, int track_index, int num_recommendations) {
    int* recommendations = (int*)malloc(num_recommendations * sizeof(int));
    float* similarities = (float*)malloc(rec->num_tracks * sizeof(float));
    
    // 获取当前曲目与其他曲目的相似度
    for (int i = 0; i < rec->num_tracks; i++) {
        similarities[i] = rec->similarity_matrix[track_index * rec->num_tracks + i];
    }
    
    // 选择最相似的曲目
    for (int i = 0; i < num_recommendations; i++) {
        float max_sim = -1.0f;
        int max_idx = -1;
        
        for (int j = 0; j < rec->num_tracks; j++) {
            if (j != track_index && similarities[j] > max_sim) {
                max_sim = similarities[j];
                max_idx = j;
            }
        }
        
        recommendations[i] = max_idx;
        similarities[max_idx] = -1.0f;  // 标记为已选择
    }
    
    free(similarities);
    return recommendations;
}

// 释放推荐系统
void free_recommender(Recommender* rec) {
    for (int i = 0; i < rec->num_tracks; i++) {
        free_music_features(rec->features[i]);
    }
    free(rec->features);
    free(rec->similarity_matrix);
    free(rec->genre_indices);
    free(rec);
} 