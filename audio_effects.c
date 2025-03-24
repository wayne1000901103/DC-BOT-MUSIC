#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fftw3.h>
#include <complex.h>

// 音频效果结构体
typedef struct {
    float bass_boost;
    float treble_boost;
    float reverb_level;
    float echo_delay;
    float echo_gain;
    float compression_threshold;
    float compression_ratio;
    float distortion_level;
    float chorus_rate;
    float chorus_depth;
    float chorus_mix;
    float flanger_rate;
    float flanger_depth;
    float flanger_mix;
    float phaser_rate;
    float phaser_depth;
    float phaser_mix;
    float wah_frequency;
    float wah_q;
    float wah_mix;
} AudioEffects;

// 音频可视化结构体
typedef struct {
    float* spectrum;
    float* waveform;
    float* energy;
    int spectrum_size;
    int waveform_size;
    float* window;
} AudioVisualizer;

// 初始化音频效果
AudioEffects* init_audio_effects() {
    AudioEffects* effects = (AudioEffects*)malloc(sizeof(AudioEffects));
    effects->bass_boost = 0.0f;
    effects->treble_boost = 0.0f;
    effects->reverb_level = 0.0f;
    effects->echo_delay = 0.0f;
    effects->echo_gain = 0.0f;
    effects->compression_threshold = -20.0f;
    effects->compression_ratio = 4.0f;
    effects->distortion_level = 0.0f;
    effects->chorus_rate = 1.0f;
    effects->chorus_depth = 0.0f;
    effects->chorus_mix = 0.0f;
    effects->flanger_rate = 1.0f;
    effects->flanger_depth = 0.0f;
    effects->flanger_mix = 0.0f;
    effects->phaser_rate = 1.0f;
    effects->phaser_depth = 0.0f;
    effects->phaser_mix = 0.0f;
    effects->wah_frequency = 1000.0f;
    effects->wah_q = 1.0f;
    effects->wah_mix = 0.0f;
    return effects;
}

// 初始化音频可视化
AudioVisualizer* init_visualizer(int spectrum_size, int waveform_size) {
    AudioVisualizer* viz = (AudioVisualizer*)malloc(sizeof(AudioVisualizer));
    viz->spectrum = (float*)calloc(spectrum_size, sizeof(float));
    viz->waveform = (float*)calloc(waveform_size, sizeof(float));
    viz->energy = (float*)calloc(spectrum_size, sizeof(float));
    viz->spectrum_size = spectrum_size;
    viz->waveform_size = waveform_size;
    
    // 创建汉宁窗
    viz->window = (float*)malloc(waveform_size * sizeof(float));
    for (int i = 0; i < waveform_size; i++) {
        viz->window[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (waveform_size - 1)));
    }
    
    return viz;
}

// 应用失真效果
void apply_distortion(float* buffer, int size, float level) {
    for (int i = 0; i < size; i++) {
        float x = buffer[i];
        float threshold = 1.0f - level;
        if (x > threshold) {
            buffer[i] = threshold + (x - threshold) * 0.5f;
        } else if (x < -threshold) {
            buffer[i] = -threshold + (x + threshold) * 0.5f;
        }
    }
}

// 应用合唱效果
void apply_chorus(float* buffer, int size, float rate, float depth, float mix) {
    float* delay_buffer = (float*)malloc(size * sizeof(float));
    memset(delay_buffer, 0, size * sizeof(float));
    
    float phase = 0.0f;
    float phase_increment = 2.0f * M_PI * rate / 44100.0f;
    
    for (int i = 0; i < size; i++) {
        float delay = depth * sinf(phase);
        int delay_samples = (int)(delay * 44100.0f);
        if (i >= delay_samples) {
            delay_buffer[i] = buffer[i - delay_samples];
        }
        phase += phase_increment;
        if (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
        
        buffer[i] = buffer[i] * (1.0f - mix) + delay_buffer[i] * mix;
    }
    
    free(delay_buffer);
}

// 应用镶边效果
void apply_flanger(float* buffer, int size, float rate, float depth, float mix) {
    float* delay_buffer = (float*)malloc(size * sizeof(float));
    memset(delay_buffer, 0, size * sizeof(float));
    
    float phase = 0.0f;
    float phase_increment = 2.0f * M_PI * rate / 44100.0f;
    
    for (int i = 0; i < size; i++) {
        float delay = depth * sinf(phase);
        int delay_samples = (int)(delay * 44100.0f);
        if (i >= delay_samples) {
            delay_buffer[i] = buffer[i - delay_samples];
        }
        phase += phase_increment;
        if (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
        
        buffer[i] = buffer[i] * (1.0f - mix) + delay_buffer[i] * mix;
    }
    
    free(delay_buffer);
}

// 应用相位器效果
void apply_phaser(float* buffer, int size, float rate, float depth, float mix) {
    float* filtered = (float*)malloc(size * sizeof(float));
    memcpy(filtered, buffer, size * sizeof(float));
    
    float phase = 0.0f;
    float phase_increment = 2.0f * M_PI * rate / 44100.0f;
    
    for (int i = 0; i < size; i++) {
        float frequency = 1000.0f * (1.0f + depth * sinf(phase));
        float omega = 2.0f * M_PI * frequency / 44100.0f;
        float alpha = sinf(omega) / (2.0f * 0.707f);
        
        if (i > 0) {
            filtered[i] = alpha * (buffer[i] - filtered[i-1]) + filtered[i-1];
        }
        
        phase += phase_increment;
        if (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
        
        buffer[i] = buffer[i] * (1.0f - mix) + filtered[i] * mix;
    }
    
    free(filtered);
}

// 应用哇音效果
void apply_wah(float* buffer, int size, float frequency, float q, float mix) {
    float* filtered = (float*)malloc(size * sizeof(float));
    memcpy(filtered, buffer, size * sizeof(float));
    
    float omega = 2.0f * M_PI * frequency / 44100.0f;
    float alpha = sinf(omega) / (2.0f * q);
    
    for (int i = 1; i < size; i++) {
        filtered[i] = alpha * (buffer[i] - filtered[i-1]) + filtered[i-1];
        buffer[i] = buffer[i] * (1.0f - mix) + filtered[i] * mix;
    }
    
    free(filtered);
}

// 更新音频可视化
void update_visualization(float* buffer, int size, AudioVisualizer* viz) {
    // 应用窗口函数
    float* windowed = (float*)malloc(size * sizeof(float));
    for (int i = 0; i < size; i++) {
        windowed[i] = buffer[i] * viz->window[i];
    }
    
    // 计算频谱
    fftwf_complex* in = fftwf_alloc_complex(size);
    fftwf_complex* out = fftwf_alloc_complex(size);
    fftwf_plan plan = fftwf_plan_dft_1d(size, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    
    for (int i = 0; i < size; i++) {
        in[i][0] = windowed[i];
        in[i][1] = 0.0f;
    }
    
    fftwf_execute(plan);
    
    // 计算频谱能量
    for (int i = 0; i < viz->spectrum_size; i++) {
        float magnitude = sqrtf(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        viz->spectrum[i] = magnitude;
        viz->energy[i] = magnitude * magnitude;
    }
    
    // 更新波形
    for (int i = 0; i < viz->waveform_size; i++) {
        viz->waveform[i] = buffer[i];
    }
    
    // 清理
    fftwf_destroy_plan(plan);
    fftwf_free(in);
    fftwf_free(out);
    free(windowed);
}

// 应用所有音频效果
void apply_effects(float* buffer, int size, AudioEffects* effects) {
    apply_bass_boost(buffer, size, effects->bass_boost);
    apply_treble_boost(buffer, size, effects->treble_boost);
    apply_reverb(buffer, size, effects->reverb_level);
    apply_echo(buffer, size, effects->echo_delay, effects->echo_gain);
    apply_compression(buffer, size, effects->compression_threshold, effects->compression_ratio);
    apply_distortion(buffer, size, effects->distortion_level);
    apply_chorus(buffer, size, effects->chorus_rate, effects->chorus_depth, effects->chorus_mix);
    apply_flanger(buffer, size, effects->flanger_rate, effects->flanger_depth, effects->flanger_mix);
    apply_phaser(buffer, size, effects->phaser_rate, effects->phaser_depth, effects->phaser_mix);
    apply_wah(buffer, size, effects->wah_frequency, effects->wah_q, effects->wah_mix);
}

// 释放音频效果
void free_audio_effects(AudioEffects* effects) {
    free(effects);
}

// 释放音频可视化
void free_visualizer(AudioVisualizer* viz) {
    free(viz->spectrum);
    free(viz->waveform);
    free(viz->energy);
    free(viz->window);
    free(viz);
} 