/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "play_audio.h"

/**
 * @brief PlayAudio类的构造函数
 */
PlayAudio::PlayAudio() {
    pthread_handle_ = 0;
    sample_rate_ = 44100;
    bit_width_ = KD_AUDIO_BIT_WIDTH_16;
    i2s_work_mode_ = K_STANDARD_MODE;
    current_audio_file_ = "";
}

/**
 * @brief PlayAudio类的析构函数
 */
PlayAudio::~PlayAudio() {
    end_audio();
}

/**
 * @brief 初始化音频系统
 */
void PlayAudio::init_audio() {
    int codec_flag = 1;
    audio_sample_enable_audio_codec((k_bool)codec_flag);
    audio_sample_vb_init(K_TRUE, sample_rate_);
}

/**
 * @brief 音频线程的执行函数
 *
 * @param arg 指向PlayAudio对象的指针
 * @return 无返回值
 */
void* PlayAudio::audio_thread_fn(void* arg) {
    PlayAudio* audio = reinterpret_cast<PlayAudio*>(arg);
    std::string wavFilePath = audio->current_audio_file_;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    printf("sample ao i2s module: %s\n", wavFilePath.c_str());
    audio_sample_send_ao_data(wavFilePath.c_str(), 0, 0, audio->sample_rate_, audio->bit_width_, audio->i2s_work_mode_);

    return NULL;
}

/**
 * @brief 播放音频文件
 *
 * @param audioFilePath 音频文件的路径
 */
void PlayAudio::play_audio(const std::string& audioFilePath) {
    if (!current_audio_file_.empty()) {
        pthread_cancel(pthread_handle_);
        pthread_join(pthread_handle_, NULL);
        current_audio_file_ = "";
    }

    init_audio();
    current_audio_file_ = audioFilePath;

    pthread_create(&pthread_handle_, nullptr, audio_thread_fn, this);
    audio_sample_exit();
    pthread_join(pthread_handle_, NULL);

    printf("destroy vb block \n");
    audio_sample_vb_destroy();
}

/**
 * @brief 结束音频播放
 */
void PlayAudio::end_audio() {
    if (!current_audio_file_.empty()) {
        pthread_cancel(pthread_handle_);
        pthread_join(pthread_handle_, NULL);
    }

    audio_sample_exit();
    audio_sample_vb_destroy();
}