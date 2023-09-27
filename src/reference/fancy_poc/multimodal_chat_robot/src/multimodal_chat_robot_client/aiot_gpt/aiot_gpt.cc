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
#include "play_audio.h"

using namespace std;


#define ANSI_COLOR_RED "\033[31m"
#define ANSI_COLOR_RESET "\033[0m"

// 全局变量用于标记程序是否需要退出
volatile sig_atomic_t g_exitFlag = 0;
std::atomic<bool> isp_stop(false);


pthread_t g_pthread_handle;
k_u32 g_sample_rate = 44100;
k_audio_bit_width g_bit_width = KD_AUDIO_BIT_WIDTH_16;
k_i2s_work_mode g_i2s_work_mode = K_STANDARD_MODE;


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

/**
 * @brief 调整图像大小并进行填充
 * 
 * @param src 输入图像
 * @param dst 输出图像
 * @param size 目标大小
 * @param fx 水平缩放因子
 * @param fy 垂直缩放因子
 * @param interpolation 插值方法
 */
void padding_resize(cv::InputArray src, cv::OutputArray &dst, cv::Size size, double fx = 0, double fy = 0, int interpolation = cv::INTER_LINEAR) 
{
    float padd_w = 0;
    float padd_h = 0;
    //这里是找出原图中边长大的那个.然后先按照边长大的那个resize,然后边长小的就填充.
    float r = std::min(float(size.width) / src.cols(), float(size.height) / src.rows());
    int inside_w = round(src.cols() * r);//
    int inside_h = round(src.rows() * r);//
    padd_w = size.width - inside_w;//padd_w和padd_h其中一个是零.
    padd_h = size.height - inside_h;
    cout<<"padd_w:"<<padd_w<<",padd_h:"<<padd_h<<endl;
    cv::resize(src, dst, cv::Size(inside_w, inside_h), fx, fy, interpolation);
    padd_w = padd_w / 2;
    padd_h = padd_h / 2;
    cout<<"padd_w:"<<padd_w<<",padd_h:"<<padd_h<<endl;
    //round函数是把一个小数四舍五入之后取整.round(2.2)=2.0000;round(2.5)=3.000;
    //外层边框填充灰色
    int top = int(round(padd_h - 0.1));
    int bottom = int(round(padd_h + 0.1));
    int left = int(round(padd_w - 0.1));
    int right = int(round(padd_w + 0.1));
    cout<<"top:"<<top<<",bottom:"<<bottom<<",left:"<<left<<",right:"<<right<<endl;
 
    cv::copyMakeBorder(dst, dst, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(114, 114, 114));//top, bottom, left, right分别表示在原图四周扩充边缘的大小
}

/**
 * @brief 视频处理函数
 * 
 * @param file_path 视频文件路径
 */
void video_proc(std::string file_path)
{
    vivcap_start();
    // 设置osd参数
    k_video_frame_info vf_info;
    void *pic_vaddr = NULL;       //osd
    memset(&vf_info, 0, sizeof(vf_info));
    vf_info.v_frame.width = osd_width;
    vf_info.v_frame.height = osd_height;
    vf_info.v_frame.stride[0] = osd_width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    block = vo_insert_frame(&vf_info, &pic_vaddr);

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
    cv::Mat ori_image = cv::imread(file_path);
    cv::Mat src_image;
    padding_resize(ori_image, src_image, cv::Size(osd_width, osd_height), 0, 0,cv::INTER_LINEAR);

    vector<cv::Mat> rgb3Channels(3);
	split(src_image, rgb3Channels);
 
 
	cv::Mat alpha_mat= cv::Mat (cv::Size(osd_width, osd_height), CV_8UC1, cv::Scalar(255));


    vector<cv::Mat> channels_4;
    channels_4.push_back(alpha_mat);         //alpha=0
	channels_4.push_back(rgb3Channels[2]);	//r
	channels_4.push_back(rgb3Channels[1]);	//g
	channels_4.push_back(rgb3Channels[0]);	//b

 
	cv::Mat osd_frame;
	merge(channels_4, osd_frame);

    memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
    // 显示通道插入帧
    kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); // K_VO_OSD0
    // printf("kd_mpi_vo_chn_insert_frame success \n");

    while (!isp_stop)
    {
        memset(&dump_info, 0, sizeof(k_video_frame_info));
        ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
        if (ret)
        {
            printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
            continue;
        }

        auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
        memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
        kd_mpi_sys_munmap(vbvaddr, size);

        ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
        
        if (ret)
        {
            printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
        }

        if (is_key_pressed() == 'q')
        {
            g_exitFlag = 1;
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

/**
 * @brief 音频线程函数
 * 
 * @param arg 线程参数，为WAV文件路径
 * @return void*
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

    std::string image_file_path = "received_image.jpg";
    std::string audio_file_Path = "audio.wav";
    std::string param_file_path = "param.txt";
    time_t image_last_modified_time = 0;
    time_t audio_last_modified_time = 0;
    time_t param_last_modified_time = 0;
    std::string currentAudioFile = "";
    is_file_updated(image_file_path, image_last_modified_time);
    is_file_updated(audio_file_Path, audio_last_modified_time);
    is_file_updated(param_file_path, param_last_modified_time);
    // 注册SIGINT信号处理程序
    signal(SIGINT, sigint_handler);
    pthread_t thread_input;
    pthread_create(&thread_input, NULL, thread_input_fn, NULL);

    while (!g_exitFlag) {

        // 如果图片更新
        if (is_file_updated(image_file_path, image_last_modified_time)) 
        {
            // 开启相应的线程函数
            std::cout << ANSI_COLOR_RED << "Image is updated." << ANSI_COLOR_RESET << std::endl;
            std::thread thread_isp(video_proc, image_file_path);
			#if defined(CONFIG_BOARD_K230_CANMV)
                std::this_thread::sleep_for(std::chrono::seconds(30));
            #else
                std::this_thread::sleep_for(std::chrono::seconds(10));
            #endif
            isp_stop = true;
            thread_isp.join();
            isp_stop = false;
        }


        // 如果参数文件被更新
        if (is_file_updated(param_file_path, param_last_modified_time)){
            // 开启相应的线程函数
            std::cout << ANSI_COLOR_RED << "parameter is updated." << ANSI_COLOR_RESET << std::endl;
            
        }


        // 如果声音文件更新
        if (is_file_updated(audio_file_Path, audio_last_modified_time)) {
            std::cout << ANSI_COLOR_RED << "Audio is updated." << ANSI_COLOR_RESET << std::endl;
            int codec_flag = 1;
            audio_sample_enable_audio_codec((k_bool)codec_flag);
            audio_sample_vb_init(K_TRUE, g_sample_rate);
            pthread_create(&g_pthread_handle, NULL, audio_thread_fn, (void*)(audio_file_Path.c_str()));
            pthread_join(g_pthread_handle, NULL);
            audio_sample_exit();
            audio_sample_vb_destroy();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (!currentAudioFile.empty()) {
        pthread_cancel(g_pthread_handle);
        pthread_join(g_pthread_handle, NULL);
    }

    audio_sample_exit();
    audio_sample_vb_destroy();


    return 0;
}