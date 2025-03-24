#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fftw3.h>

// 音频效果结构体
typedef struct {
    float bass_boost;
    float treble_boost;
    float reverb_level;
    float echo_delay;
    float echo_gain;
    float compression_threshold;
    float compression_ratio;
} AudioEffects;

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
    return effects;
}

// 应用低音增强
void apply_bass_boost(float* buffer, int size, float boost) {
    for (int i = 0; i < size; i++) {
        buffer[i] *= (1.0f + boost);
    }
}

// 应用高音增强
void apply_treble_boost(float* buffer, int size, float boost) {
    fftwf_complex* in = fftwf_alloc_complex(size);
    fftwf_complex* out = fftwf_alloc_complex(size);
    fftwf_plan plan = fftwf_plan_dft_1d(size, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // 复制数据到FFT输入
    for (int i = 0; i < size; i++) {
        in[i][0] = buffer[i];
        in[i][1] = 0.0f;
    }

    // 执行FFT
    fftwf_execute(plan);

    // 增强高频
    for (int i = size/2; i < size; i++) {
        out[i][0] *= (1.0f + boost);
        out[i][1] *= (1.0f + boost);
    }

    // 执行逆FFT
    fftwf_plan reverse_plan = fftwf_plan_dft_1d(size, out, in, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftwf_execute(reverse_plan);

    // 复制结果回缓冲区
    for (int i = 0; i < size; i++) {
        buffer[i] = in[i][0] / size;
    }

    // 清理
    fftwf_destroy_plan(plan);
    fftwf_destroy_plan(reverse_plan);
    fftwf_free(in);
    fftwf_free(out);
}

// 应用混响效果
void apply_reverb(float* buffer, int size, float level) {
    float* delay_buffer = (float*)malloc(size * sizeof(float));
    memset(delay_buffer, 0, size * sizeof(float));

    for (int i = 0; i < size; i++) {
        float delayed = (i > 0) ? delay_buffer[i-1] : 0.0f;
        delay_buffer[i] = buffer[i] * 0.5f + delayed * 0.5f;
        buffer[i] += delayed * level;
    }

    free(delay_buffer);
}

// 应用回声效果
void apply_echo(float* buffer, int size, float delay, float gain) {
    int delay_samples = (int)(delay * 44100); // 假设采样率为44.1kHz
    float* delay_buffer = (float*)malloc(size * sizeof(float));
    memset(delay_buffer, 0, size * sizeof(float));

    for (int i = 0; i < size; i++) {
        if (i >= delay_samples) {
            buffer[i] += buffer[i - delay_samples] * gain;
        }
    }

    free(delay_buffer);
}

// 应用压缩效果
void apply_compression(float* buffer, int size, float threshold, float ratio) {
    for (int i = 0; i < size; i++) {
        float level = 20 * log10(fabs(buffer[i]));
        if (level > threshold) {
            float gain_reduction = (level - threshold) * (1 - 1/ratio);
            buffer[i] *= pow(10, -gain_reduction/20);
        }
    }
}

// 应用所有音频效果
void apply_effects(float* buffer, int size, AudioEffects* effects) {
    apply_bass_boost(buffer, size, effects->bass_boost);
    apply_treble_boost(buffer, size, effects->treble_boost);
    apply_reverb(buffer, size, effects->reverb_level);
    apply_echo(buffer, size, effects->echo_delay, effects->echo_gain);
    apply_compression(buffer, size, effects->compression_threshold, effects->compression_ratio);
}

// 释放音频效果
void free_audio_effects(AudioEffects* effects) {
    free(effects);
} 