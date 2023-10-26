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

#include <iostream>
#include <chrono>
#include <thread>
#include <sys/stat.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <signal.h>
#include <termios.h> 
#include <fcntl.h>
#include <sys/types.h>

#include "utils.h"
#include "vi_vo.h"
#include "hand_detection.h"
#include "hand_keypoint.h"
#include "mpi_sys_api.h"
#include "audio_sample.h"
#include "ocr_box.h"
#include "ocr_reco.h"
#include "sort.h"

#define ANSI_COLOR_RED "\033[31m"
#define ANSI_COLOR_RESET "\033[0m"


// 全局变量用于标记程序是否需要退出
volatile sig_atomic_t g_exitFlag = 0;


/**
 * @brief SIGINT信号处理程序
 *
 * @param signum 信号编号
 */
void sigint_handler(int signum)
{
    // 设置退出标志
    g_exitFlag = 1;
    std::exit(0);
}

/**
 * @brief 检查文件是否已更新
 *
 * @param filePath 文件路径
 * @param lastModifiedTime 上次修改时间
 * @return 如果文件已更新，则返回true；否则返回false
 */
bool is_file_updated(const std::string& filePath, time_t& lastModifiedTime)
{
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) != 0) {
        std::cout << "Failed to get file information: " << filePath << std::endl;
        return false;
    }

    time_t modifiedTime = fileInfo.st_mtime;
    if (modifiedTime != lastModifiedTime) {
        lastModifiedTime = modifiedTime;
        return true;
    }

    return false;
}

/**
 * @brief 检查是否有键盘按键按下
 * 
 * @return 如果有按键按下，则返回相应按键的ASCII码；否则返回0
 */
int is_key_pressed()
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    // 获取终端设置
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    // 设置终端为非规范模式，即禁用行缓冲
    newt.c_lflag &= ~(ICANON | ECHO);
    // 将更改应用于终端
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    // 获取终端标志
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    // 将终端标志设置为非阻塞模式
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    // 从终端读取一个字符
    ch = getchar();

    // 还原终端设置和标志
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    // 如果读取到字符，则返回该字符，否则返回0
    if (ch != EOF)
        return ch;
    else
        return 0;
}

/**
 * @brief 输入线程函数，用于检测键盘输入并设置退出标志
 * 
 * @param arg 传递给线程的参数（此处未使用）
 * @return void* 
 */
void* thread_input_fn(void*)
{
    while (!g_exitFlag)
    {
        // 检查是否有按键按下，如果有按下 'q' 键，则设置退出标志
        if (is_key_pressed() == 'q')
        {
            g_exitFlag = 1;
        }

        // 短暂休眠，避免过多消耗CPU资源
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return NULL;
}

std::atomic<bool> isp_stop(false);
pthread_t isp_thread_handle;
pthread_t g_pthread_handle;
k_u32 g_sample_rate = 44100;
k_audio_bit_width g_bit_width = KD_AUDIO_BIT_WIDTH_16;
k_i2s_work_mode g_i2s_work_mode = K_STANDARD_MODE;
std::atomic<bool> is_block(false);

/**
 * @brief 阻塞函数，用来定格检测到的手指及OCR内容 
 */
void block_func(){
    
    while(1){
        if (!is_block){
            break;
        }
    };
    
}

// 自定义比较函数，根据meanx和meany坐标排序多边形
bool comparePolygons(const Boxb& a, const Boxb& b) {
    // 按照从上到下、从左到右的顺序进行排序
    if (a.meany < b.meany) 
    {
        return true;
    } 
    else if (a.meany > b.meany) 
    {
        return false;
    } 
    else
    {
        return a.meanx < b.meanx;
    }
}

// 示例函数，排序results中的多边形
void sortPolygons(std::vector<Boxb>& results) {
    std::sort(results.begin(), results.end(), comparePolygons);
}

/**
 * @brief OCR处理函数，用于OCR检测及OCR识别
 *
 * @param ori_img 原始图片
 * @param osd_frame_img osd显示图片
 * @param xmin 用来标识手指包围框在原图左上角坐标x
 * @param ymin 用来标识手指包围框在原图右上角坐标y
 * 
 * @return 如果文件已更新，则返回true；否则返回false
 */
int ocr_process(OCRBox *ocrbox, OCRReco *ocrreco,cv::Mat ori_img, cv::Mat& osd_frame_img,int xmin, int ymin)
{
    cv::Mat draw_img = ori_img.clone();
    int ori_w = ori_img.cols;
    int ori_h = ori_img.rows;

    ocrbox->pre_process(ori_img);

    ocrbox->inference();

    vector<Boxb> results;
    ocrbox->post_process({ori_w, ori_h}, results);
    if(results.size() == 0)
    {
        std::ofstream ocr_file("ocr.txt");
        if (ocr_file.is_open()) {
            // 写入指定的文本
            ocr_file << "什么都没有";
            ocr_file.close();
        } 
        else 
        {
            std::cout << "Failed to open ocr.txt for writing." << std::endl;
        }
        std::cout<<line<<std::endl;
    }
    else
    {
        sortPolygons(results);
        string ocr_line;

        for(int i = 0; i < results.size(); i++)
        {
            vector<Point> vec;
            vec.clear();
            for(int j = 0; j < 4; j++)
            {
                vec.push_back(results[i].vertices[j]);
            }
            cv::RotatedRect rect = cv::minAreaRect(vec);
            cv::Point2f ver[4];
            rect.points(ver);
            cv::Mat crop;
            Utils::warppersp(ori_img, crop, results[i]);

            ocrreco->pre_process(crop);
            ocrreco->inference();

            vector<string> results2;
            
            ocrreco->post_process(results2);
            for (int i=0;i<results2.size();i++)
            {
                // std::cout<<results2[i]<<endl;
                ocr_line += results2[i];
            }
        }

        std::ofstream ocr_file("ocr.txt");
        if (ocr_file.is_open()) {
            // 写入指定的文本
            ocr_file << ocr_line;
            ocr_file.close();
        } 
        else 
        {
            std::cout << "Failed to open ocr.txt for writing." << std::endl;
        }
        std::cout<<ocr_line<<std::endl;
    
        for(int i = 0; i < results.size(); i++)
        {   
            std::vector<cv::Point> vec;
            vec.clear();
            for(int j = 0; j < 4; j++)
            {
                cv::Point tmp = results[i].vertices[j];
                tmp.x = 1.0*(tmp.x+xmin)/SENSOR_WIDTH*osd_width;
                tmp.y = 1.0*(tmp.y+ymin)/SENSOR_HEIGHT*osd_height;

                vec.push_back(tmp);
            }
            cv::RotatedRect rect = minAreaRect(vec);
            cv::Point2f ver[4];
            rect.points(ver);
            for(int i = 0; i < 4; i++)
                line(osd_frame_img, ver[i], ver[(i + 1) % 4], Scalar(255, 0, 0, 0), 3);
        }
    }
    return results.size();
}

/**
 * @brief 视频流+手势检测+OCR识别流程函数
 * 
 */
void video_proc(HandDetection *hd, HandKeypoint *hk, OCRBox *ocrbox, OCRReco *ocrreco)
{
    vivcap_start();
    // 设置osd参数
    k_video_frame_info vf_info;
    void *pic_vaddr = NULL; // osd
    memset(&vf_info, 0, sizeof(vf_info));
    vf_info.v_frame.width = osd_width;
    vf_info.v_frame.height = osd_height;
    vf_info.v_frame.stride[0] = osd_width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    block = vo_insert_frame(&vf_info, &pic_vaddr);

    int debug_mode = 1;

    // alloc memory,get isp memory
    size_t paddr = 0;
    void *vaddr = nullptr;
    size_t size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", size);
    if (ret)
    {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }
 
    hd->set_vaddr_(reinterpret_cast<uintptr_t>(vaddr));
    hk->set_vaddr_(reinterpret_cast<uintptr_t>(vaddr));

    {
        std::vector<BoxInfo> results;

        Sort sort;
        int fi = 0;

        int dumped_img = -1;
        cv::Mat cropped_img;
        int ocr_det_size = -1;

        while (!isp_stop)
        {
            ScopedTiming st("total time", 1);
            {
                ScopedTiming st("read capture", debug_mode);
                // 从vivcap中读取一帧图像到dump_info
                memset(&dump_info, 0, sizeof(k_video_frame_info));
                ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
                if (ret)
                {
                    printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                    continue;
                }
            }

            if(dumped_img<=0)
            {
                {
                    ScopedTiming st("isp copy", debug_mode);
                    auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
                    memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
                    kd_mpi_sys_munmap(vbvaddr, size);
                }

                results.clear();

                hd->pre_process();
                hd->inference();
                // 旋转后图像
                hd->post_process(results);

                std::vector<Sort::TrackingBox> frameTrackingResult = sort.Sortx(results, fi);
                fi ++;

                cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
                for (auto r: results)
                {
                    std::string text = hd->labels_[r.label] + ":" + std::to_string(round(r.score * 100) / 100.0);
                    std::cout << "text = " << text << std::endl;

                    int w = r.x2 - r.x1 + 1;
                    int h = r.y2 - r.y1 + 1;
                    
                    int rect_x = r.x1/ SENSOR_WIDTH * osd_width;
                    int rect_y = r.y1 / SENSOR_HEIGHT  * osd_height;
                    int rect_w = (float)w / SENSOR_WIDTH * osd_width;
                    int rect_h = (float)h / SENSOR_HEIGHT  * osd_height;
                    cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255, 255), 2, 2, 0);
                }

                if (frameTrackingResult.size()>=2 && fi>=3)
                {
                    cv::Point2f left_top, right_bottom;

                    int index1 = 1;
                    int index2 = 1;

                    std::ifstream file("param.txt");
                    std::vector<int> numbers;
                    std::string line, token;

                    if (file.is_open()) 
                    {
                        std::getline(file, line);
                        std::stringstream ss(line);

                        while (std::getline(ss, token, ',')) 
                        {
                            int num;
                            std::istringstream(token) >> num;
                            numbers.push_back(num);
                        }

                        file.close();

                        for (int num : numbers) 
                        {
                            std::cout << "Finger Number: " << num << std::endl;
                        }
                    } 
                    else 
                    {
                        std::cout << "Failed to open the file." << std::endl;
                    }

                    index1 = numbers[0];
                    index2 = numbers[1];

                    for(int i=0;i< frameTrackingResult.size();i++)
                    {
                        auto tb = frameTrackingResult[i];

                        int rect_x = tb.box.x / SENSOR_WIDTH * osd_width;
                        int rect_y = tb.box.y / SENSOR_HEIGHT  * osd_height;
                        int rect_w = (float)tb.box.width / SENSOR_WIDTH * osd_width;
                        int rect_h = (float)tb.box.height / SENSOR_HEIGHT  * osd_height;
                        cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w + 20, rect_h + 20), cv::Scalar( 255,255, 0, 255), 2, 2, 0);

                        std::string num = std::to_string(tb.id);

                        int length = std::max(tb.box.width,tb.box.height)/2;
                        int cx = tb.box.x+tb.box.width/2;
                        int cy = tb.box.y+tb.box.height/2;
                        int ratio_num = 1.26*length;

                        int x1_1 = std::max(0,cx-ratio_num);
                        int y1_1 = std::max(0,cy-ratio_num);
                        int x2_1 = std::min(SENSOR_WIDTH-1, cx+ratio_num);
                        int y2_1 = std::min(SENSOR_HEIGHT-1, cy+ratio_num);
                        int w_1 = x2_1 - x1_1 + 1;
                        int h_1 = y2_1 - y1_1 + 1;
                        
                        struct Bbox bbox = {x:x1_1,y:y1_1,w:w_1,h:h_1};
                        hk->pre_process(bbox);

                        hk->inference();

                        float *pred = hk->get_out()[0];
                        int draw_x,draw_y;
                        if (i==0)
                        {
                            float left_pred_x = std::max(std::min(pred[(index1+1)*4*2], 1.0f), 0.0f);
                            float left_pred_y = std::max(std::min(pred[(index1+1)*4*2+1], 1.0f), 0.0f);

                            left_top.x = left_pred_x * w_1 + x1_1;
                            left_top.y = left_pred_y * h_1 + y1_1;
                            
                            draw_x = left_top.x / SENSOR_WIDTH * osd_width;
                            draw_y = left_top.y / SENSOR_HEIGHT * osd_height;
                        }
                        if (i==1)
                        {
                            float right_pred_x = std::max(std::min(pred[(index2+1)*4*2], 1.0f), 0.0f);
                            float right_pred_y = std::max(std::min(pred[(index2+1)*4*2+1], 1.0f), 0.0f);

                            right_bottom.x = right_pred_x * w_1 + x1_1;
                            right_bottom.y = right_pred_y * h_1 + y1_1;
                        
                            draw_x = right_bottom.x / SENSOR_WIDTH * osd_width;
                            draw_y = right_bottom.y / SENSOR_HEIGHT * osd_height;
                        }
                        cv::circle(osd_frame, cv::Point(draw_x, draw_y), 6, cv::Scalar(255, 0,0,0), 3);
                        cv::circle(osd_frame, cv::Point(draw_x, draw_y), 5, cv::Scalar(255, 0,0,0), 3);

                        ScopedTiming st("osd draw", debug_mode);
                        hk->draw_keypoints(osd_frame, num, bbox, false);
                    }

                    int x_min = std::min(left_top.x, right_bottom.x);
                    int x_max = std::max(left_top.x, right_bottom.x);
                    int y_min = std::min(left_top.y, right_bottom.y);
                    int y_max = std::max(left_top.y, right_bottom.y);
                    Bbox box_info = {x_min,y_min, (x_max-x_min+1),(y_max-y_min+1)};

                    if(dumped_img <= 0)
                    {
                        dumped_img = 1;
                        is_block = true;
                        int matsize = SENSOR_WIDTH * SENSOR_HEIGHT;
                        
                        cv::Mat ori_img;
                        cv::Mat ori_img_R = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr);
                        cv::Mat ori_img_G = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr + 1 * matsize);
                        cv::Mat ori_img_B = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr + 2 * matsize);
                        std::vector<cv::Mat> sensor_rgb;
                        sensor_rgb.push_back(ori_img_B);
                        sensor_rgb.push_back(ori_img_G);
                        sensor_rgb.push_back(ori_img_R);
                        cv::merge(sensor_rgb, ori_img);

                        cropped_img = Utils::crop(ori_img, box_info);
                        cv::imwrite("./cropped_img.png", cropped_img);

                        {
                            std::cout<<"ocr start!" <<endl;
                            ocr_det_size = ocr_process(ocrbox,ocrreco,cropped_img,osd_frame,x_min,y_min);
                        }
                        std::cout<<"ocr done!" <<endl;

                    }

                    int x = 1.0 * x_min/ SENSOR_WIDTH * osd_width;
                    int y = 1.0 * y_min / SENSOR_HEIGHT  * osd_height;
                    int w = 1.0 * (x_max-x_min) / SENSOR_WIDTH * osd_width;
                    int h = 1.0 * (y_max-y_min) / SENSOR_HEIGHT  * osd_height;
                    cv::rectangle(osd_frame, cv::Rect(x, y , w, h), cv::Scalar( 255,0, 255, 255), 2, 2, 0);

                    {
                        cv::Mat tmp_mat_R, tmp_mat_G, tmp_mat_B;
                        
                        int matsize = SENSOR_WIDTH * SENSOR_HEIGHT;
                        cv::Mat ori_img_R = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr);
                        cv::Mat ori_img_G = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr + 1 * matsize);
                        cv::Mat ori_img_B = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr + 2 * matsize);
 
                        cv::resize(ori_img_R, tmp_mat_R, cv::Size(osd_width, osd_height), cv::INTER_AREA);
                        cv::resize(ori_img_G, tmp_mat_G, cv::Size(osd_width, osd_height), cv::INTER_AREA);
                        cv::resize(ori_img_B, tmp_mat_B, cv::Size(osd_width, osd_height), cv::INTER_AREA);

                        uint8_t *p_r_addr = reinterpret_cast<uint8_t *>(tmp_mat_R.data);
                        uint8_t *p_g_addr = reinterpret_cast<uint8_t *>(tmp_mat_G.data);
                        uint8_t *p_b_addr = reinterpret_cast<uint8_t *>(tmp_mat_B.data);

                        for(uint32_t hh = 0; hh < osd_height; hh++)
                        {
                            for(uint32_t ww = 0; ww < osd_width; ww++)
                            {
                                int new_hh = hh;
                                int new_ww = ww;
                                int osd_channel_index = (new_hh * osd_width + new_ww) * 4;
                                if(osd_frame.data[osd_channel_index + 0] == 0)                        
                                {
                                    int ori_pix_index = hh * osd_width + ww;
                                    osd_frame.data[osd_channel_index + 0] = 255;
                                    osd_frame.data[osd_channel_index + 1] =  p_r_addr[ori_pix_index];
                                    osd_frame.data[osd_channel_index + 2] =  p_g_addr[ori_pix_index];
                                    osd_frame.data[osd_channel_index + 3] =  p_b_addr[ori_pix_index]; 
                                }                        
                            }
                        }
                    }
                }
                {
                    ScopedTiming st("osd copy", debug_mode);
                    memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
                    // 显示通道插入帧
                    kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); // K_VO_OSD0
                }
            }
            else
            {
                {
                    ScopedTiming st("osd clear", debug_mode);
                    cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
                    memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
                    // 显示通道插入帧
                    kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); // K_VO_OSD0
                }
                
            }
            
            {
                ScopedTiming st("vicap release", debug_mode);
                ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
                if (ret)
                {
                    printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                }
            }
            block_func();
        }
    }    

    vo_osd_release_block();
    vivcap_stop();

    // free memory
    ret = kd_mpi_sys_mmz_free(paddr, vaddr);
    if (ret)
    {
        std::cerr << "free failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }
}

struct ThreadArgs {
    HandDetection *hd;
    HandKeypoint *hk;
    OCRBox *ocrbox;
    OCRReco *ocrreco;
};

/**
 * @brief video_proc线程启动函数
 * 
 */
void* isp_thread_fn(void* args)
{
    ThreadArgs* threadArgs = static_cast<ThreadArgs*>(args);
    
    // Access the objects inside the thread function
    HandDetection *hd = threadArgs->hd;
    HandKeypoint *hk = threadArgs->hk;
    OCRBox *ocrbox = threadArgs->ocrbox;
    OCRReco *ocrreco = threadArgs->ocrreco;
    video_proc(hd, hk, ocrbox, ocrreco);
    return nullptr;
}

/**
 * @brief 声音播放线程启动函数
 * 
 */
void* audio_thread_fn(void* arg)
{
    std::string wavFilePath = static_cast<char*>(arg);

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    printf("sample ao i2s module: %s\n", wavFilePath.c_str());
    audio_sample_send_ao_data(wavFilePath.c_str(), 0, 0, g_sample_rate, g_bit_width, g_i2s_work_mode);
    return NULL;
}


int main() {
    std::string audio_file_Path = "audio.wav";
    std::string param_file_path = "param.txt";

    time_t audio_last_modified_time = 0;
    time_t param_last_modified_time = 0;

    is_file_updated(audio_file_Path, audio_last_modified_time);
    is_file_updated(param_file_path, param_last_modified_time);
    // 注册SIGINT信号处理程序
    signal(SIGINT, sigint_handler);

    size_t paddr = 0;
    void *vaddr = nullptr;    
    const char* hand_det_kmodel = "hand_det.kmodel";
    const char* handkp_det_kmodel = "handkp_det.kmodel";
    float obj_thresh = 0.15;
    float nms_thresh = 0.4;
    int debug_mode = 1;

    HandDetection *hd = new HandDetection(hand_det_kmodel, obj_thresh, nms_thresh, {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), debug_mode);
    HandKeypoint *hk = new HandKeypoint(handkp_det_kmodel, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), debug_mode);
    const char* ocr_det_model = "ocr_det.kmodel";
    float threshold_seg = 0.15;
    float threshold_box = 0.4;
    const char* ocr_rec_model = "ocr_rec.kmodel";
    // int debug_mode = 1;

    OCRBox *ocrbox = new OCRBox(ocr_det_model, threshold_seg, threshold_box,  debug_mode);
    OCRReco *ocrreco = new OCRReco(ocr_rec_model,6625,debug_mode);  //96   
    ThreadArgs threadArgs;
    threadArgs.hd = hd;
    threadArgs.hk = hk;
    threadArgs.ocrbox = ocrbox;
    threadArgs.ocrreco = ocrreco;

    while (1) {

        // 如果参数文件被更新
        if (is_file_updated(param_file_path, param_last_modified_time)){
            // 开启相应的线程函数
            std::cout << ANSI_COLOR_RED << "parameter is updated." << ANSI_COLOR_RESET << std::endl;

            pthread_create(&isp_thread_handle, NULL, isp_thread_fn, &threadArgs);
            
            std::cout<<"reader start!"<<std::endl;
        }

        // 如果声音文件更新
        if (is_file_updated(audio_file_Path, audio_last_modified_time)) {
            std::cout << ANSI_COLOR_RED << "Audio is updated." << ANSI_COLOR_RESET << std::endl;
            isp_stop = true;
            is_block = false;
            pthread_join(isp_thread_handle, NULL);
            isp_stop = false;
            std::cout<<"finger process end！"<<std::endl;
            int codec_flag = 1;
            audio_sample_enable_audio_codec((k_bool)codec_flag);
            audio_sample_vb_init(K_TRUE, g_sample_rate);
            pthread_create(&g_pthread_handle, NULL, audio_thread_fn, (void*)(audio_file_Path.c_str()));
            pthread_join(g_pthread_handle, NULL);
           
            audio_sample_exit();
            audio_sample_vb_destroy();
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    delete hd;
    delete hk;
    delete ocrbox;
    delete ocrreco;

    return 0;
}
