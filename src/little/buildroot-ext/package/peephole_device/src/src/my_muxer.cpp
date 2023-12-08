#include "my_muxer.h"
#include <cstring>
#include <iostream>
#include <memory>
#include "mp4_format.h"

volatile int storage_ready_ = 0;

class Mp4Muxer {
  public:
    Mp4Muxer() {}
    int Open(const std::string &filename) {
        k_mp4_config_s mp4_config;
        memset(&mp4_config, 0, sizeof(mp4_config));
        mp4_config.config_type = K_MP4_CONFIG_MUXER;
        strcpy(mp4_config.muxer_config.file_name, filename.c_str());
        mp4_config.muxer_config.fmp4_flag = 1;

        int ret = kd_mp4_create(&mp4_muxer_, &mp4_config);
        if (ret < 0) {
            std::cout << "mp4 muxer create failed" << std::endl;
            kd_mp4_destroy(mp4_muxer_);
            return -1;
        }

        k_mp4_track_info_s video_track_info;
        k_mp4_track_info_s audio_track_info;

        memset(&video_track_info, 0, sizeof(video_track_info));
        video_track_info.track_type = K_MP4_STREAM_VIDEO;
        video_track_info.time_scale = 1000;
        video_track_info.video_info.width = 1088;
        video_track_info.video_info.height = 1920;
        ret = kd_mp4_create_track(mp4_muxer_, &video_track_handle_, &video_track_info);
        if (ret < 0){
            std::cout << "create video track failed." << std::endl;
            kd_mp4_destroy_tracks(mp4_muxer_);
            kd_mp4_destroy(mp4_muxer_);
            return -1;
        }
        memset(&audio_track_info, 0, sizeof(audio_track_info));
        audio_track_info.track_type = K_MP4_STREAM_AUDIO;
        audio_track_info.time_scale = 1000;
        audio_track_info.audio_info.channels = 1;
        audio_track_info.audio_info.codec_id = K_MP4_CODEC_ID_G711U;
        audio_track_info.audio_info.sample_rate = 8000;
        audio_track_info.audio_info.bit_per_sample = 16;
        ret = kd_mp4_create_track(mp4_muxer_, &audio_track_handle_, &audio_track_info);
        if (ret < 0) {
            std::cout << "create audio track failed." << std::endl;
            kd_mp4_destroy_tracks(mp4_muxer_);
            kd_mp4_destroy(mp4_muxer_);
            return -1;
        }
        video_arrived_ = false;
        return 0;
    }

    int Close() {
        int ret = kd_mp4_destroy_tracks(mp4_muxer_);
        if (ret < 0) {
            std::cout << "destroy mp4 tracks failed.\n" << std::endl;
        }

        ret = kd_mp4_destroy(mp4_muxer_);
        if (ret < 0) {
            std::cout << "destroy mp4 failed." << std::endl;
        }
        std::cout << "mp4 file generated" << std::endl;
        return 0;
    }

    int Write(const MyMp4Muxer::AVEncData &data) {
        using DataType = AVEncFrameData::Type;
        bool is_video = false;
        k_mp4_codec_id_e codec_id;
        if (data.type == DataType::H264) codec_id = K_MP4_CODEC_ID_H264, is_video = true;
        else if(data.type == DataType::H265) codec_id = K_MP4_CODEC_ID_H265, is_video = true;
        else if(data.type == DataType::PCMU) codec_id = K_MP4_CODEC_ID_G711U, is_video = false;
        else {
            std::cout << "codec type not supported yet" << std::endl;
            return 1;
        }

        k_mp4_frame_data_s frame_data;
        memset(&frame_data, 0, sizeof(frame_data));
        frame_data.codec_id = codec_id;
        frame_data.data = (uint8_t *)data.data.get();
        frame_data.data_length = data.size;
        frame_data.time_stamp = data.timestamp_ms * 1000; // to meet mp4-muxer's requirement

        if (is_video) {
            if (video_sequence_ != data.sequence) {
                std::cout << "DEBUG: mp4muxer::write -- video sequence not expected: " << data.sequence << ", expect : " << video_sequence_ << std::endl;
                video_sequence_ = data.sequence;
            }
            ++video_sequence_;

            // if(!video_arrived_) frame_data.time_stamp = 0;
            int ret = kd_mp4_write_frame(mp4_muxer_, video_track_handle_, &frame_data);
            if (ret < 0) {
                std::cout << "mp4 write video frame failed." << std::endl;
                return -1;
            }
            video_arrived_ = true;
        } else {
            if (!video_arrived_) return 0;
            if (audio_sequence_ != data.sequence) {
                std::cout << "DEBUG: mp4muxer::write -- audio sequence not expected: " << data.sequence << ", expect : " << audio_sequence_ << std::endl;
                audio_sequence_ = data.sequence;
            }
            ++audio_sequence_;

            int ret = kd_mp4_write_frame(mp4_muxer_, audio_track_handle_, &frame_data);
            if (ret < 0) {
                printf("mp4 write audio frame failed.\n");
                return -1;
            }
        }
        return 0;
    }

  private:  // emphasize the following members are private    
    Mp4Muxer(const Mp4Muxer &);
    const Mp4Muxer &operator=(const Mp4Muxer &);

  private:
    KD_HANDLE mp4_muxer_ = NULL;
    KD_HANDLE video_track_handle_ = NULL;
    KD_HANDLE audio_track_handle_ = NULL;
    uint32_t video_sequence_{0};
    uint32_t audio_sequence_{0};
    bool video_arrived_ = false;
};

int MyMp4Muxer::Init(IMp4MuxerCallback *cb) {
    if(!cb) return -1;
    std::unique_lock<std::mutex> lck(mutex_);
    if (initialized_) return 0;
    callback_ = cb;
    exit_flag_.store(false);
    force_exit_.store(false);
    start_process_ = false;
    thread_ = std::thread([this](){
        this->Process();
    });
    initialized_ = true;
    return 0;
}

void MyMp4Muxer::DeInit(bool force) {
    std::unique_lock<std::mutex> lck(mutex_);
    if (!initialized_) return;
    if (thread_.joinable()) {
        force_exit_.store(force);
        exit_flag_.store(true);
        thread_.join();
    }
    start_process_ = false;
    initialized_ = false;
}

int MyMp4Muxer::Write(const AVEncFrameData &frame_data) {
/*
    static int count = 0;
    if(++count < 32) {
        std::cout << "-------------------------------------------" << std::endl;
        std::cout << "frame_type = " << static_cast<int>(frame_data.type) << std::endl;
        std::cout << "size = " << frame_data.size << std::endl;
        std::cout << "timestamp = " << (long)frame_data.timestamp_ms << std::endl;
        std::cout << "keyframe = " << (long)frame_data.keyframe << std::endl;
        std::cout << "sequence = " << (long)frame_data.sequence << std::endl;
        std::cout << "flags = " << (long)frame_data.flags << std::endl;
    }
*/
    std::unique_lock<std::mutex> lck(mutex_);
    if (!initialized_) return -1;
    if (frame_data.flags == 0x1000 || frame_data.flags == 0x1001) {
        AVEncData data;
        data.flags = frame_data.flags;
        std::unique_lock<std::mutex> qlck(q_mutex_);
        queue_.push(data);
        return 0;
    }

    // TODO,  adjust timestamp for avsync ...
    //
    AVEncData data;
    data.type = frame_data.type;
    data.size = frame_data.size;
    data.timestamp_ms = frame_data.timestamp_ms;
    data.keyframe = frame_data.keyframe;
    data.sequence = frame_data.sequence;
    data.flags = frame_data.flags;
    std::shared_ptr<uint8_t> p(new uint8_t[data.size], std::default_delete<uint8_t[]>());
    memcpy(p.get(), frame_data.data, data.size);
    data.data = p;
    std::unique_lock<std::mutex> qlck(q_mutex_);
    queue_.push(data);
    start_process_ = true; // TODO
    return 0;
}

/*
void MyMp4Muxer::StartProcess() {
    std::unique_lock<std::mutex> lck(q_mutex_);
    start_process_ = true;
}
*/

void MyMp4Muxer::Process() {
    if (!callback_) {
        return;
    }

    Mp4Muxer muxer;
    bool muxer_opened = false;
    bool keyframe_got = false;
    bool enable_record = false;
    int count = 0;

    while (!exit_flag_) {
        AVEncData data;
        {
            std::unique_lock<std::mutex> lck(q_mutex_);
            if (!start_process_ || queue_.empty()) {
                lck.unlock();
                usleep(1000 * 10);
                continue;
            }
            // FIXME, to control memory used by stream data
            if (!storage_ready_) {
                lck.unlock();
                usleep(1000 * 100);
                continue;
            }

            data = queue_.top();
            queue_.pop();
        }

        if (!callback_->IsMp4SpaceAvailable()) {
            // std::cout << "MyMp4Muxer::Process, no space, discard data" << std::endl;
            usleep(1000 * 10);
            continue;
        }

        // check cmd first
        if(data.flags == 0x1000) {
            std::cout << "MyMp4Muxer::Process, receive command to stop record" << std::endl;
            enable_record = false;
            if (muxer_opened) {
                muxer.Close();
                muxer_opened = false;
                std::cout << "MyMp4Muxer::Process, muxer.Close Done" << std::endl;
            }
            continue;
        } else if (data.flags == 0x1001) {
            enable_record = true;
            muxer_opened = false;
            std::cout << "MyMp4Muxer::Process, receive command to start record" << std::endl;
            continue;
        }

        if (!enable_record) {
            usleep(1000 * 10);
            continue;
        }

        // record is enabled, process data
        if (!muxer_opened) {
            std::string filename;
            callback_->GetMp4Filename(filename);
            if (filename.empty()) {
                std::cout << "MyMp4Muxer::Process, failed to get filename" << std::endl;
                return;
            }
            if(muxer.Open(filename) < 0) {
                return;
            }
            muxer_opened = true;
            keyframe_got = false;
            count = 0;
            std::cout << "MyMp4Muxer::Process, new mp4 file created: " << filename << std::endl;
        }

        if (muxer_opened) {
            if (!keyframe_got) {
                if (data.type != AVEncFrameData::Type::PCMU && data.keyframe) {
                    keyframe_got = true;
                }
            }
            if (!keyframe_got) {
                usleep(1000);
                continue;
            }
            if(++count < 8) {
                std::cout << "-------------------------------------------" << std::endl;
                std::cout << "frame_type = " << static_cast<int>(data.type) << std::endl;
                std::cout << "size = " << data.size << std::endl;
                std::cout << "timestamp = " << (long)data.timestamp_ms << std::endl;
                std::cout << "keyframe = " << (long)data.keyframe << std::endl;
                std::cout << "sequence = " << (long)data.sequence << std::endl;
                std::cout << "flags = " << (long)data.flags << std::endl;
                }
            int ret = muxer.Write(data);
            if (ret < 0) {
                break;
            }
        }
        usleep(1000);
    }

    std::cout << "MyMp4Muxer::Process  exit ..." << std::endl;
    std::unique_lock<std::mutex> lck(q_mutex_);
    while(!queue_.empty()) {
        if (!force_exit_) {
            auto data = queue_.top();
            if (data.flags == 0x1000 || data.flags == 0x1001) {
                queue_.pop();
                continue;
            }
            if (muxer_opened) {
                if( muxer.Write(data) < 0) {
                    std::cout << "mp4 muxer.Write failed" << std::endl;
                    force_exit_ = true;
                }
            }
        }
        queue_.pop();
    }
    if (muxer_opened) {        
        muxer.Close();
        std::cout << "MyMp4Muxer::Process, muxer.Close Done" << std::endl;
    }
    std::cout << "MyMp4Muxer::Process Exit" << std::endl;
}

// JpegMuxer
int MyJpegMuxer::Init(IJpegMuxerCallback *cb) {
    std::unique_lock<std::mutex> lck(mutex_);
    if(initialized_) return 0;
    callback_ = cb;
    exit_flag_.store(false);
    thread_ = std::thread([this](){
        this->Process();
    });
    initialized_ = true;
    return 0;
}

void MyJpegMuxer::DeInit(bool force) {
    std::unique_lock<std::mutex> lck(mutex_);
    if(!initialized_) return;
    if (thread_.joinable()) {
        force_exit_.store(force);
        exit_flag_.store(true);
        thread_.join();
    }
    initialized_ = false;
}

int MyJpegMuxer::Write(const UserEventData &event) {
    std::unique_lock<std::mutex> lck(mutex_);
    if (!initialized_) return -1;

    EventData data;
    data.type = event.type;
    data.timestamp_ms = event.timestamp_ms;
    data.jpeg_size = event.jpeg_size;
    std::shared_ptr<uint8_t> p(new uint8_t[data.jpeg_size], std::default_delete<uint8_t[]>());
    memcpy(p.get(), event.jpeg, event.jpeg_size);
    data.jpeg = p;
    std::unique_lock<std::mutex> qlck(q_mutex_);
    queue_.push(data);
    return 0;
}

void MyJpegMuxer::Process() {
    if (!callback_) return;
    auto save_jpeg = [this](const EventData &data) {
        std::string filename;
        callback_->GetJpegFilename(data.type, data.timestamp_ms, filename);
        if (filename.empty()) {
            std::cout << "failed to jpeg filename" << std::endl;
            return -1;
        }
        std::cout << "JpegMuxer::Process() -- filename : " << filename << std::endl;

        FILE *fp = fopen(filename.c_str(), "wb");
        if (fp) {
            fwrite(data.jpeg.get(), 1, data.jpeg_size, fp);
            fclose(fp);
        } else {
            std::cout << "failed to save jpeg file: " << filename << std::endl;
            return -1;
        }
        return 0;
    };

    while (!exit_flag_) {
        EventData data;
        {
            std::unique_lock<std::mutex> lck(q_mutex_);
            if (queue_.empty()) {
                lck.unlock();
                usleep(1000 * 10);
                continue;
            }

            // FIXME, to control memory used by jpeg data
            if (!storage_ready_) {
                lck.unlock();
                // std::cout << "jpeg muxer --- waiting for storage ready ... " << std::endl;
                usleep(1000 * 100);
                continue;
            }

            data = queue_.front();
            queue_.pop();
        }

        if (!callback_->IsJpegSpaceAvailable()) {
            usleep(1000 * 10);
            continue;
        }

        if (save_jpeg(data) < 0) {
            break;
        }
        usleep(1000 * 10);
    }

    std::unique_lock<std::mutex> lck(q_mutex_);
    while(!queue_.empty()) {
        if(!force_exit_) {
            if(save_jpeg(queue_.front()) < 0) {
                std::cout << "jpeg muxer.Write failed" << std::endl;
                force_exit_ = true;
            }
        }
        queue_.pop();
    }
    std::cout << "MyJpegMuxer::Process Exit" << std::endl;
}
