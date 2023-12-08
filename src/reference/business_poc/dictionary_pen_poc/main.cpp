#include "tts_api.h"
#include "stitch_api.hpp"
#include "ocr_api.h"

#include "nmt_api.h"
#include "CvxText.h"
#include "stitch.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <atomic>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>

#include <unistd.h>
#include <pthread.h>
#include "sys/ioctl.h"
#include <stdio.h>
#include "stdio.h"
#include "k_autoconf_comm.h"

#include "vo.h"
#include "audio_buf_play.h"

#include <queue>


using namespace std;

// .ocr_api ./image_path ./det_kmodel ./ocr_kmodel ./dict_path ./result_path
// ./stitch ./test_data_dir   ./result_path

// osd_cidianbi.plane cidianbi.plane;

cidianbi_attr cidianbi;
Mat stich_picture;
k_video_frame_info dump_info;
k_video_frame_info release_info;

queue <k_video_frame_info> video_frame;


bool app_run = true;

ocr_api ocr_api;
tts_api tts_api;
nmt_api nmt_api;

static k_s32 sample_vb_init(k_bool enable_cache, k_u32 sample_rate)
{

    k_s32 ret;
    k_vb_config config;
    k_vb_pool_config pool_config;
    k_u32 pool_id;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;

    config.comm_pool[1].blk_cnt = 150;
    config.comm_pool[1].blk_size = sample_rate * 2 * 4 ;// AUDIO_PERSEC_DIV_NUM;
    config.comm_pool[1].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

    config.comm_pool[2].blk_cnt = 2;
    config.comm_pool[2].blk_size = sample_rate * 2 * 4 ;//AUDIO_PERSEC_DIV_NUM * 2; // ao use
    config.comm_pool[2].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

    //VB for YUV420SP output
    config.comm_pool[3].blk_cnt = VICAP_MAX_FRAME_COUNT;
    config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[3].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3 / 2), VICAP_ALIGN_1K);

    config.comm_pool[0].blk_cnt = PRIVATE_POLL_NUM;
    config.comm_pool[0].blk_size = PRIVATE_POLL_SZE; 
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;

    int blk_total_size = 0;
    for (int i = 0; i < 3; i++)
    {
        blk_total_size += config.comm_pool[i].blk_cnt * config.comm_pool[i].blk_size;
    }
    printf("mmz blk total size:%.2f MB\n", blk_total_size / 1024 / 1024.0);

    ret = kd_mpi_vb_set_config(&config);
    if (ret)
    {
        printf("vb_set_config failed ret:%d\n", ret);
        return ret;
    }
    else
    {
        printf("vb_set_config ok\n");
    }

    ret = kd_mpi_vb_init();

    if (ret)
        printf("vb_init failed ret:%d\n", ret);
    
    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = PRIVATE_POLL_NUM;
    pool_config.blk_size = PRIVATE_POLL_SZE;
    pool_config.mode = VB_REMAP_MODE_NONE;
    pool_id = kd_mpi_vb_create_pool(&pool_config);      // osd0 - 3 argb 320 x 240
    cidianbi.plane.g_pool_id = pool_id;

    printf("cidianbi.plane.g_pool_id is %d \n", cidianbi.plane.g_pool_id);

    return ret;
}

static void sample_display_init(k_vo_osd chn)
{
    osd_info osd;
    k_vo_osd osd_id = chn;

    osd.act_size.width = 1080 ;
    osd.act_size.height = 480;
    osd.offset.x = 0;
    osd.offset.y = 800;
    osd.global_alptha = 0xff;
    osd.format = PIXEL_FORMAT_ARGB_8888;//PIXEL_FORMAT_ARGB_4444; //PIXEL_FORMAT_ARGB_1555;//PIXEL_FORMAT_ARGB_8888;

    memcpy(&cidianbi.plane.osd, &osd, sizeof(osd));

    sample_connector_init(HX8377_V2_MIPI_4LAN_1080X1920_30FPS);

    // sample_vo_creat_private_poll();
    // config osd
    sample_vo_creat_osd(osd_id, &osd);

#if VICAP_VO_TESET 
    sample_vo_layer_config();
#endif

}

static void *exit_app(void *arg)
{
    printf("press 'q' to exit application!!\n");
    while(getchar() != 'q')
    {
        usleep(10000);
        if(app_run == false)
            break;
    }
    app_run = false;
    printf("exit_app start exit  \n");
    return NULL;
}

static bool key_status = 0;
static void *gpio_app(void *arg)
{
    pin_mode_t key0;
    pin_mode_t key1;
    pin_mode_t led0;
    pin_mode_t led1;
    int val0, val1;
    int gpio_fd = -1;
    key0.pin = KEY_PIN_NUM1;
    key1.pin = KEY_PIN_NUM2;

    led0.pin = LED_PIN_NUM1;
    led1.pin = LED_PIN_NUM2;

    cidianbi.key_val = 0;

    gpio_fd = sample_gpio_init();
    ioctl(gpio_fd, GPIO_WRITE_LOW, &led0);
    ioctl(gpio_fd, GPIO_WRITE_LOW, &led1);

    while(app_run)
    {
        
        ioctl(gpio_fd, GPIO_READ_VALUE, &key0);
        ioctl(gpio_fd, GPIO_READ_VALUE, &key1);
        val0 = key0.mode;
        val1 = key1.mode;
        if(!key_status)
        {
            if (!val0 || !val1)     //任意键按下皆可开灯
            {
                ioctl(gpio_fd, GPIO_WRITE_HIGH, &led0);
                ioctl(gpio_fd, GPIO_WRITE_HIGH, &led1);
                key_status = 1;
                pthread_mutex_lock(&cidianbi.mutex_key);
                cidianbi.key_val = 1;
                pthread_mutex_unlock(&cidianbi.mutex_key);
            }
        }
        else
        {
            if (val0 && val1)   //两键同时松开才关灯
            {
                ioctl(gpio_fd, GPIO_WRITE_LOW, &led0);
                ioctl(gpio_fd, GPIO_WRITE_LOW, &led1);
                key_status = 0;
                pthread_mutex_lock(&cidianbi.mutex_key);
                cidianbi.key_val = 0;
                pthread_mutex_unlock(&cidianbi.mutex_key);
            }
        }
        usleep(10 * 1000); //delay 10ms 
    }

    cidianbi.exit_flag = cidianbi.exit_flag | (1 << 0);

    printf("gpio thread exit success cidianbi.exit_flag is %x \n", cidianbi.exit_flag);

    return NULL;
}

static bool is_vicap_open(void)
{
    int ret = 0;

    ret = kd_mpi_vicap_dump_frame(cidianbi.vicap_dev, VICAP_CHN_ID_0, VICAP_DUMP_YUV, &dump_info, 1000);
    if (ret) {
        printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
        sample_vivcap_deinit();
        return 0;
    }

    ret = kd_mpi_vicap_dump_release(cidianbi.vicap_dev, VICAP_CHN_ID_0, &dump_info);
    if (ret) {
        printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
    }  

    return 1;
}

static void *sample_vicap_dump_picture(void *arg)
{
    int ret;
    int save_picture_flag = 0;
    size_t size = ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3 / 2;
    int is_switch = 0;
    bool is_open = 0;

    // stith picture 
    Mat current_image_grey;//读当前图片
    Mat left_image_history;//初始化，用于中间结果
    
    /// 定义变量                                                    // 当前帧的图像路                                                                                                                                                   // 累计拼接图像
    int last_right_start_x = 0;                                    // 记录上一次拼接时，右图在累计拼接图像中的起始坐标(左上角的x坐标)   参数3
    int last_right_start_y = 0;                                    // 记录上一次拼接时，右图在累计拼接图像中的起始坐标(左上角的y坐标)   参数4
    int round = 0;                                                 // 处理的第几帧图像
    int flag_start = 0;                                            // 是否开始进行拼接
    int flag_stop = 0;                                             // 是否停止拼接                                                                                                                                           // 历史待匹配的左图
    string debug_save_prefix;
    /// 定长输出相关的变量
    int interval_length = 0;//默认为0
    //int interval_length = 0;     // 当拼接图像的长度大于该值，则输出拼接图像，并重新开始拼接。设为<0时，为非定长输出。
    // int interval_length = 200;  // 当拼接图像的长度大于该值，则输出拼接图像，并重新开始拼接。设为<0时，为非定长输出。
    int interval_flag_save = 0;
    Mat image_stitch;//初始化，用于最终结果

    sample_vivcap_init();

    int i = 0;
    for(i = 0; i < 3; i++)
    {
        is_open = is_vicap_open();
        if(is_open == 0)
            sample_vivcap_init();
        else
            break;
    }

    while(app_run)
    {
        pthread_mutex_lock(&cidianbi.mutex_key);
        int curt_val = cidianbi.key_val;
        if((save_picture_flag == 1) && (curt_val == 0))
        {
            is_switch = 1;
        }
        save_picture_flag = cidianbi.key_val;
        pthread_mutex_unlock(&cidianbi.mutex_key);
        if(save_picture_flag == 1)
        {
            memset(&dump_info, 0 , sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(cidianbi.vicap_dev, VICAP_CHN_ID_0, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                break;
            }
            // save picture 
            // video_frame.push(dump_info);
//            printf("sample_vicap...kd_mpi_vicap_dump_frame success. video_frame deep is %d  %x \n", video_frame.size(), dump_info.v_frame.phys_addr[0]);

            auto vbvaddr = kd_mpi_sys_mmap(dump_info.v_frame.phys_addr[0], size);

            current_image_grey = cv::Mat(ISP_CHN0_HEIGHT, ISP_CHN0_WIDTH, CV_8UC(1), vbvaddr);
            if(current_image_grey.empty()){
                cout << "error: stitch input image is empty: " << endl;
                continue;
            }
            stitch(current_image_grey, left_image_history, image_stitch, last_right_start_x, last_right_start_y, round, flag_start, flag_stop, debug_save_prefix,
                interval_length, interval_flag_save);

            // imwrite("vicap_stitch.png", image_stitch);

            ret = kd_mpi_vicap_dump_release(cidianbi.vicap_dev, VICAP_CHN_ID_0, &dump_info);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
            }  
            kd_mpi_sys_munmap(vbvaddr, size);

            // release_info = video_frame.front();
            // memcpy(&release_info, &video_frame.front(), sizeof(release_info));
            // video_frame.pop();
            // auto vbvaddr = kd_mpi_sys_mmap(release_info.v_frame.phys_addr[0], size);
            // kd_mpi_sys_munmap(vbvaddr, size);

            // printf("sample_vicap...kd_mpi_vicap_dump_release success. video_frame deep is %d release_info addr i %x \n", video_frame.size(), release_info.v_frame.phys_addr[0]);
            // ret = kd_mpi_vicap_dump_release(cidianbi.vicap_dev, VICAP_CHN_ID_0, &release_info);
            // if (ret) {
            //     printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
            // }         
        }
        else   
        {
            if(is_switch == 1)
            {
                is_switch = 0;
            //    imwrite("over_vicap_stitch.png", image_stitch); // save picture 
            //    printf("save picture success --------------------- \n");

                pthread_mutex_lock(&cidianbi.stitch);
                cidianbi.stitch_val = 1;
                stich_picture = image_stitch.clone();
                pthread_mutex_unlock(&cidianbi.stitch);

                // imwrite("over2_vicap_stitch.png", image_stitch); // save picture S

                current_image_grey.release();
                left_image_history.release();
                image_stitch.release();
                debug_save_prefix.clear();
                last_right_start_x = 0;
                last_right_start_y = 0;
                round = 0;
                flag_start = 0;
                flag_stop = 0;
                interval_length = 0;
                interval_flag_save = 0;
            }
            usleep(10000);          // delay 10ms
        }
            
    }

    sample_vivcap_deinit();
#if VICAP_VO_TESET
    kd_mpi_vo_disable_video_layer(K_VO_LAYER1);
#endif
    cidianbi.exit_flag = cidianbi.exit_flag | (1 << 1);

    printf("vicap_dump_picture thread exit success cidianbi.exit_flag is %x \n", cidianbi.exit_flag);
    return NULL;
}


#define PICTURE_DIR         "./testdata/Fri_Mar_24_00_47_10_2023_"

static void *cidianbi_programming(void *arg)
{
    k_video_frame_info vf_info;
    void *pic_vaddr = NULL;
    k_vb_blk_handle block;
    k_u8 *audio_pdata;
    k_u32 audio_len;
    string input_text;
    string nmt_string;
    Mat current_image_grey;
    
    //audio init 
    audio_buffer_play_init(16000, 1);

    // set frame
    memset(&vf_info, 0, sizeof(vf_info));
    vf_info.v_frame.width = cidianbi.plane.osd.act_size.width;
    vf_info.v_frame.height = cidianbi.plane.osd.act_size.height;
    vf_info.v_frame.stride[0] = cidianbi.plane.osd.act_size.width;
    vf_info.v_frame.pixel_format = cidianbi.plane.osd.format;
    block = sample_vo_insert_frame(&vf_info, &pic_vaddr);

    printf("cidianbi_programming------------------inin success ----------- \n");
    while(app_run)
    {
        if(cidianbi.stitch_val == 1)
        {
            printf("cidianbi_programming----------------------------- \n");
            pthread_mutex_lock(&cidianbi.stitch);
            cidianbi.stitch_val = 0;
            pthread_mutex_unlock(&cidianbi.stitch);

            input_text.clear();
            nmt_string.clear();

            // stitch_api(PICTURE_DIR,"./result_stitch.jpg");
            // current_image_grey = imread("./result_stitch.jpg", 0);
            input_text = ocr_api.ocr_run(stich_picture);
            
            imwrite("ocr_stitch.png", stich_picture); // save picture 

            std::vector<uint8_t> audio_data;
            bool flag = tts_api.tts_run(input_text, audio_data);
            nmt_string = nmt_api.nmt_run(input_text,flag);
            
            sample_vo_filling_color(pic_vaddr, input_text, nmt_string, flag);
            kd_mpi_vo_chn_insert_frame(cidianbi.chn + 3, &vf_info);  //K_VO_OSD0
            
            audio_pdata = (k_u8*)&audio_data[0];
            k_u32 audio_len = audio_data.size();
            audio_buffer_play(audio_pdata, audio_len);
            sleep(1);
        }
        else
        {
            usleep(10000);
        }
        
    }

     // close cidianbi.plane
    kd_mpi_vo_osd_disable(cidianbi.chn);
    audio_buffer_play_deinit();
    kd_mpi_vb_release_block(block);
    kd_mpi_vb_destory_pool(cidianbi.plane.g_pool_id);
    cidianbi.exit_flag = cidianbi.exit_flag | (2 << 1);

    printf("cidianbi_programming thread exit success cidianbi.exit_flag is %x \n", cidianbi.exit_flag);

    return NULL;
}

void fun_sig(int sig)
{
    if(sig == SIGINT)
    {
        printf("recive ctrl+c\n");
        app_run = false;
        // quit.store(false);
    }
}

int main(void) 
{
    int ret = 0;
    k_u32 g_max_sample_rate = 48000;

    printf("start add model \n");

    memset(&cidianbi, 0, sizeof(cidianbi));
    memset(&cidianbi.plane, 0, sizeof(cidianbi.plane));

    /****fixed operation for ctrl+c****/
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = fun_sig;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    //加载kmodel
    ocr_api.load_model();
    tts_api.load_model();
    nmt_api.load_model();

    cidianbi.chn = K_VO_OSD3;
    cidianbi.exit_flag = 0;
    //vb init
    sample_vb_init(K_TRUE, g_max_sample_rate);

    pthread_t vo_thread_handle;
    pthread_t exit_thread_handle;
    pthread_t gpio_thread_handle;
    pthread_t dump_picture_thread_handle;
    pthread_t cidianbi_thread_handle;
    pthread_t stitch_thread_handle;

    // init thread 
    pthread_mutex_init(&cidianbi.mutex_key, NULL);
    pthread_mutex_init(&cidianbi.stitch, NULL);

    // display init 
    sample_display_init(cidianbi.chn);

    pthread_create(&exit_thread_handle, NULL, exit_app, NULL);                  
    pthread_create(&gpio_thread_handle, NULL, gpio_app, NULL);                               // ext_flag = bit[0] = 1    0x1
    pthread_create(&dump_picture_thread_handle, NULL, sample_vicap_dump_picture, NULL);      // ext_flag = bit[1] = 1    0x2
    pthread_create(&cidianbi_thread_handle, NULL, cidianbi_programming, NULL);                  // ext_flag = bit[2] = 1    0x4
    //  pthread_create(&stitch_thread_handle, NULL, stitch_programming, NULL);                  // ext_flag = bit[3] = 1    0x8

    while(cidianbi.exit_flag != 0x7)  // 0x1 , 0x2,  0x4 
    {
        sleep(1);
    }

    pthread_join(exit_thread_handle, NULL); 
    pthread_join(gpio_thread_handle, NULL);
    pthread_join(dump_picture_thread_handle, NULL);
    pthread_join(cidianbi_thread_handle, NULL);
    // pthread_join(stitch_thread_handle, NULL);
    
    kd_mpi_vb_exit();

    printf("exit success \n");

    return 0;
}
