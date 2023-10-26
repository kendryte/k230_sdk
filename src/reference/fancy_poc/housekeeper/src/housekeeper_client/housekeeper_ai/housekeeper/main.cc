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
#include <unordered_map>
#include <vector>
#include <string>
#include <thread>
#include <dirent.h>
#include <fstream> 
#include "person_detect.h"
#include "BYTETracker.h"
#include "face_detection.h"
#include "face_pose.h"
#include "face_recognition.h"
#include "falldown_detect.h"
#include "json.h"
#include "utils.h"
#include "vi_vo.h"

using namespace std;
#define face_iou_threshold 0.9
#define body_iou_threshold 0.7

std::atomic<bool> isp_stop(false);


void print_usage(const char *name)
{
    cout << "Usage: " << name << " <kmodel> <pd_thresh> <nms_thresh> <input_mode> <debug_mode> <fps> <buffer>" << endl
         << "For example: " << endl
         << " [for isp] ./bytetrack.elf bytetrack_yolov5n.kmodel 0.5 0.45 None 0 24 30" << endl
         << " [for img] ./bytetrack.elf bytetrack_yolov5n.kmodel 0.5 0.45 277 0 24 30" << endl
         << "Options:" << endl
         << " 1> kmodel_bytetrack    bytetrack行人检测kmodel文件路径 \n"
         << " 2> pd_thresh  行人检测阈值 \n"
         << " 3> nms_thresh_bytetrack   NMS阈值 \n"
         << " 4> input_mode       图像 (Number) or 摄像头(None) \n"
         << " 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试 \n"
         << " 6> fps         帧率 \n" 
         << " 7> buffer      容忍帧数，即超过多少帧之后无法匹配上某个track，就认为该track丢失 \n"
         << " 8> kmodel_felldown    摔倒检测kmodel文件路径 \n"
         << " 9> fdd_thresh  摔倒检测阈值\n"
         << " 10> nms_thresh_felldown  NMS阈值\n"
         << " 11> db_img_dir               数据库图像路径\n" 
         << " 12> kmodel_det               人脸检测kmodel路径\n"
         << " 13> obj_thres                人脸检测阈值\n"
         << " 14> nms_thres_face                人脸检测nms阈值\n"
         << " 15> kmodel_recg              人脸识别kmodel路径\n"
         << " 16> max_register_face        人脸识别数据库最大容量\n"
         << " 17> recg_thres               人脸识别阈值\n"
         << " 18> kmodel_fp                人脸姿态估计kmodel路径\n"
         << " 19> roll_thres               人脸姿态估计滚转角,转头角度\n"
         << " 20> yaw_thres                人脸姿态估计偏航角,摇头角度\n"
         << " 21> pitch_thres              人脸姿态估计偏航角,抬/低头角度\n"
         << "\n"
         << endl;
}

/**
 * @brief 计算两个人体检测框的交并比
 *
 * @param 第一个人体检测框
 * @param 第二个人体检测框
 * @return 两个人体检测框的交并比
 */
float calculateIoU(Rect rect1, Rect rect2) {
    float xA = std::max(rect1.x, rect2.x);
    float yA = std::max(rect1.y, rect2.y);
    float xB = std::min(rect1.x + rect1.width- 1, rect2.x + rect2.width- 1);
    float yB = std::min(rect1.y + rect1.height - 1, rect2.y + rect2.height - 1);
    float intersectionArea = std::max(0.0f, xB - xA+ 1) * std::max(0.0f, yB - yA+ 1);
    float areaRect1 = rect1.width * rect1.height;
    float areaRect2 = rect2.width * rect2.height;
    float unionArea = areaRect1 + areaRect2 - intersectionArea;
    float iou = intersectionArea / unionArea;
    return iou;
}

/**
 * @brief 计算人脸检测框与人体检测框重叠部分占人脸检测框的比例
 *
 * @param 人体检测框
 * @param 人脸检测框
 * @return 人脸检测框与人体检测框重叠部分占人脸检测框的比例
 */
float calculateIoU_face_body(Rect rect1, Rect rect2) {
    float xA = std::max(rect1.x, rect2.x);
    float yA = std::max(rect1.y, rect2.y);
    float xB = std::min(rect1.x + rect1.width- 1, rect2.x + rect2.width- 1);
    float yB = std::min(rect1.y + rect1.height- 1, rect2.y + rect2.height- 1);
    float intersectionArea = std::max(0.0f, xB - xA+ 1) * std::max(0.0f, yB - yA+ 1);
    float areaRect2 = rect2.width * rect2.height;
    float iou = intersectionArea / areaRect2;
    return iou;
}

/**
 * @brief 保存文件
 *
 * @param 保存文件的名称
 * @param 保存的内容
 * @return 无返回值，会将需要保存的内容保存为指定名称文件
 */
void save_file(string filename, std::unordered_map<std::string, std::string> task) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) 
    {
        for (const auto& entry : task) 
            outFile << entry.first << " " << entry.second << "\n";
        outFile.close();
    } 
    else 
        std::cout << "Unable to open the file for writing." << std::endl;
}

void video_proc(Json::Value argv)
{   
    vivcap_start();
    k_video_frame_info vf_info;
    void *pic_vaddr = NULL;  
    memset(&vf_info, 0, sizeof(vf_info));
    vf_info.v_frame.width = osd_width;
    vf_info.v_frame.height = osd_height;
    vf_info.v_frame.stride[0] = osd_width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    block = vo_insert_frame(&vf_info, &pic_vaddr);
    size_t paddr = 0;
    void *vaddr = nullptr;
    size_t size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", size);
    if (ret)
    {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }
    //定义和加载模型
    personDetect pd(argv["kmodel_bytetrack"].asCString(), argv["pd_thresh"].asFloat(),argv["nms_thresh_bytetrack"].asFloat(), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), argv["debug_mode"].asInt());
    falldownDetect fdd(argv["kmodel_felldown"].asCString(), argv["fdd_thresh"].asFloat(),argv["nms_thresh_felldown"].asFloat(), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), argv["debug_mode"].asInt());
    FaceDetection fd(argv["kmodel_det"].asCString(), argv["obj_thres"].asFloat(),argv["nms_thres_face"].asFloat(), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), argv["debug_mode"].asInt());
    FaceRecognition face_recg(argv["kmodel_recg"].asCString(),argv["max_register_face"].asInt(),argv["recg_thres"].asFloat(), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), argv["debug_mode"].asInt());
    FacePose fp(argv["kmodel_fp"].asCString(), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), argv["debug_mode"].asInt());
    //实例化模型参数阈值
    int fps = argv["fps"].asInt();
    int buffer = argv["buffer"].asInt();
    int frame = 0;
    float recg_thres = argv["recg_thres"].asFloat() * 100;
    float roll_thres = argv["roll_thres"].asFloat();
    float yaw_thres = argv["yaw_thres"].asFloat();
    float pitch_thres = argv["pitch_thres"].asFloat();
    int max_register_face = argv["max_register_face"].asInt();
    face_recg.database_init(argv["db_img_dir"].asCString());

    std::vector<BoxInfo> results_track;
    std::vector<Object> objects;
    vector<FaceDetectionInfo> results_fd;
    vector<FaceDetectionInfo> det_results;
    std::vector<BoxInfo> results_down;
    BYTETracker tracker(fps, buffer);
    FaceRecognitionInfo recg_result;
    

    Json::Value appearance;
    Json::Value appearance_flag;
    Json::Value HeadPose_flag;
    Json::Value Face_flag;
    Json::Value Activity_flag;
    Json::Value Duration_flag;
    Json::Value Face_stop;
    Json::Value Head_pose_dict;
    Json::Value Name_dict;
    Json::Value Disappearance_count;
    Json::Value appearance_frame;
    Json::Value head_pose_begin_frame;
    Json::Value remind_count;
    Json::Value Action;

    std::unordered_map<std::string, std::string> dynamic_events = {
        {"name" , ""},
        {"activity" , ""}
    };
    std::unordered_map<std::string, std::string> tts_task = {
        {"contents" , ""}
    };
    std::unordered_map<std::string, std::string> register_info = {
        {"name", ""},
        {"role", ""},
        {"useless", "useless_info"}
    };

    while (!isp_stop)   
    {   
        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        
        std::vector<STrack> output_stracks;
        frame = frame + 1;

        {
            memset(&dump_info, 0 , sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                continue;
            }
        }

        {
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  
            kd_mpi_sys_munmap(vbvaddr, size);
        }
        //人体检测
        pd.pre_process();
        pd.inference();
        pd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results_track);

        for (auto res : results_track)
        {
            Object obj{ {res.x1,res.y1,res.x2- res.x1,res.y2- res.y1},res.label,res.score };
            objects.push_back(obj);
        }

        {
            output_stracks = tracker.update(objects);
            results_track.clear();
            objects.clear();
        }

        for (Json::Value::iterator it = appearance.begin(); it != appearance.end(); ++it) 
            appearance[it.key().asString()] = false;

        //对检测到的每个人进行单独处理
        for (int i = 0; i < output_stracks.size(); i++)
        {   
            std::string track_id = std::to_string(output_stracks[i].track_id);
            std::vector<float> tlwh = output_stracks[i].tlwh;
            appearance[track_id] = true;
            bool vertical = tlwh[2] / tlwh[3] > 1.6;
            //初始化track_id对应人的各个flag参数
            if (appearance_flag[track_id].isNull()){
                appearance_flag[track_id]=false;
                HeadPose_flag[track_id]=false;
                Face_flag[track_id]=false;
                Activity_flag[track_id]=false;
                Duration_flag[track_id]=false;
                Face_stop[track_id]=false;
                Head_pose_dict[track_id]=true;
                Name_dict[track_id]="none";

                Action[track_id]="NoFall";
                Disappearance_count[track_id]=0;
            }
            
            if (tlwh[2] * tlwh[3] > 20 && !vertical)
            {
                Scalar color_tracker = tracker.get_color(output_stracks[i].track_id);
                int x_track =  tlwh[0] / SENSOR_WIDTH * osd_width;
                int y_track =  tlwh[1] / SENSOR_HEIGHT  * osd_height;
                int w_track = tlwh[2] / SENSOR_WIDTH * osd_width;
                int h_track = tlwh[3] / SENSOR_HEIGHT  * osd_height;
                
                if(Name_dict[track_id]=="none" || Name_dict[track_id]=="unknown" )
                {
                    //人脸检测
                    fd.pre_process();
                    fd.inference();
                    fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results_fd);
                    for (int i = 0; i < results_fd.size(); ++i)
                    {
                        int x_face = int(results_fd[i].bbox.x);
                        int y_face = int(results_fd[i].bbox.y);
                        int w_face = results_fd[i].bbox.w;
                        int h_face = results_fd[i].bbox.h;
                        float value_iou=calculateIoU_face_body(Rect(tlwh[0],tlwh[1],tlwh[2],tlwh[3]),Rect(x_face,y_face,w_face,h_face));
                        if (value_iou > face_iou_threshold)
                        {   
                            //人脸姿态角检测，不满足姿态角则不进行人脸识别
                            fp.pre_process(results_fd[i].bbox);
                            fp.inference();
                            FacePoseInfo pose_result;
                            fp.post_process(pose_result);
                            Head_pose_dict[track_id]=true;
                            if(abs(pose_result.roll)>roll_thres || abs(pose_result.yaw)>yaw_thres || abs(pose_result.pitch)>pitch_thres)
                                Head_pose_dict[track_id] = false;
                            if (Head_pose_dict[track_id]==true)
                            {   
                                //人脸识别
                                face_recg.pre_process(results_fd[i].sparse_kps.points);
                                face_recg.inference();
                                face_recg.database_search(recg_result);
                                if(recg_result.score>recg_thres)
                                    Name_dict[track_id] = recg_result.name;
                                else
                                    Name_dict[track_id] = "unknown";
                                break;
                            }
                        }
                    }
                }
                
                //摔倒检测
                fdd.pre_process();
                fdd.inference();
                fdd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results_down);

                for (auto r : results_down)
                {
                    int x_down = r.x1 / SENSOR_WIDTH * osd_width;
                    int y_down = r.y1 / SENSOR_HEIGHT  * osd_height;
                    int w_down = (r.x2-r.x1) / SENSOR_WIDTH * osd_width;
                    int h_down = (r.y2-r.y1) / SENSOR_HEIGHT  * osd_height;
                    float value_iou=calculateIoU(Rect(x_track,y_track,w_track,h_track),Rect(x_down,y_down,w_down,h_down));
                    if (value_iou > body_iou_threshold){
                        Action[track_id] = fdd.labels[r.label];
                        break;
                    }
                }

                results_down.clear();
                results_fd.clear();
                //画检测框+模型运行结果
                std::string Text="name:"+Name_dict[track_id].asString()+" ||"+Action[track_id].asString();
                cv::putText(osd_frame, Text, Point(x_track, y_track-5), 0, 2,  (255,0, 255, 255), 2, LINE_AA);
                cv::rectangle(osd_frame, Rect(x_track,y_track,w_track,h_track), color_tracker, 2);
            }
        }

        {
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            kd_mpi_vo_chn_insert_frame(osd_id+3, &vf_info);
            ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
            if (ret)
                printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
        }

        //逻辑判断
        for (Json::Value::iterator it = appearance_flag.begin(); it != appearance_flag.end(); ++it) 
        {
            std::string track_id_str = it.key().asString();
            if (appearance[track_id_str] == true) {
                //但凡有人出现就得置零。
                Disappearance_count[track_id_str] = 0;
                //任务一：第一次有人出现时，将Appearance_flag设为true，为后面的消失判断做准备。
                if (appearance_flag[track_id_str] == false) 
                {
                    appearance_flag[track_id_str] = true;
                    appearance_frame[track_id_str] = frame;
                    tts_task["contents"] = "您好，请面向摄像头以辨认您的身份。";
                    remind_count[track_id_str] = 0;
                    save_file("tts_task.txt",tts_task);
                    //这100帧是从当前帧到用户接收到信息作出反应的时间。
                    head_pose_begin_frame[track_id_str] = frame + 5;
                    continue;
                }
                //任务二：判断头部姿态角是否符合要求。
                if (HeadPose_flag[track_id_str] == false && frame > head_pose_begin_frame[track_id_str].asInt()) 
                {   

                    if (Head_pose_dict[track_id_str] == true && Name_dict[track_id_str] != "none")
                    {   
                        HeadPose_flag[track_id_str] = true;
                        Face_stop[track_id_str] = true;  
                        tts_task["contents"] = "谢谢您的配合！";
                        save_file("tts_task.txt",tts_task);
                        continue;
                    }
                    else 
                    {
                        tts_task["contents"] = "请正视摄像头！";
                        remind_count[track_id_str] = remind_count[track_id_str].asInt() + 1;
                        if (remind_count[track_id_str].asInt() % 15 == 0)
                            save_file("tts_task.txt",tts_task);
                        continue;
                    }
                }
                //任务三：识别到人脸后生成动态JSON。
                if (Face_flag[track_id_str] == false && Name_dict[track_id_str] != "none") 
                {
                    Face_flag[track_id_str] = true;
                    dynamic_events["name"] = Name_dict[track_id_str].asString();
                    dynamic_events["activity"] = "Just walk home.";
                    save_file("dynamic_events.txt",dynamic_events);
                    if (Name_dict[track_id_str] == "unknown" && Head_pose_dict[track_id_str] == true && face_recg.valid_register_face_ < max_register_face) 
                    {   
                        //人脸注册
                        face_recg.database_insert(argv["db_img_dir"].asCString());
                        string unknown_name =face_recg.names_[face_recg.valid_register_face_-1];
                        Name_dict[track_id_str] = unknown_name;
                        register_info["name"] =  unknown_name;
                        register_info["role"] = "guest";
                        save_file("register_info.txt",register_info);
                    }
                    continue;
                }
                //任务四：摔倒生成动态JSON。
                if (Activity_flag[track_id_str] == false && Action[track_id_str] == "Fall" && Name_dict[track_id_str] != "none") 
                {
                    Activity_flag[track_id_str] = true;
                    dynamic_events["name"] = Name_dict[track_id_str].asString();
                    dynamic_events["activity"] = "Fall down.";
                    save_file("dynamic_events.txt",dynamic_events);
                    continue;
                }
                //任务五：长时间逗留生成动态JSON。
                if (Duration_flag[track_id_str] == false && frame - appearance_frame[track_id_str].asInt() > 100 && Name_dict[track_id_str] != "none") {
                    Duration_flag[track_id_str] = true;
                    dynamic_events["name"] = Name_dict[track_id_str].asString();
                    dynamic_events["activity"] = "Standing for a long time.";
                    save_file("dynamic_events.txt",dynamic_events);
                    continue;
                }
            } 
            else
            {
                Disappearance_count[track_id_str] = Disappearance_count[track_id_str].asInt()  + 1;
                if (appearance_flag[track_id_str] == true && Disappearance_count[track_id_str] > 10  && Name_dict[track_id_str] != "none") 
                {
                    appearance_flag[track_id_str] = false;
                    Face_stop[track_id_str] = false;
                    HeadPose_flag[track_id_str] = false;
                    Duration_flag[track_id_str] = false;
                    Activity_flag[track_id_str] = false;
                    Face_flag[track_id_str] = false;
                    dynamic_events["name"] = Name_dict[track_id_str].asString();
                    dynamic_events["activity"] = "Leave home.";
                    Name_dict[track_id_str] = "none";
                    save_file("dynamic_events.txt",dynamic_events);
                    continue;
                }
            }
        }
    }

    vo_osd_release_block();
    vivcap_stop();
    ret = kd_mpi_sys_mmz_free(paddr, vaddr);
    if (ret)
    {
        std::cerr << "free failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }
}


int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;

    std::ifstream jsonFile(argv[1]);
    if (!jsonFile.is_open()) {
        std::cerr << "Failed to open JSON file." << std::endl;
        return 1;
    }

    // 创建JSON解析器
    Json::Reader reader;
    Json::Value config;

    // 解析JSON数据
    if (!reader.parse(jsonFile, config)) {
        std::cerr << "Failed to parse JSON data." << std::endl;
        return 1;
    }

    
    std::thread thread_isp(video_proc, config);
    while ( getchar() != 'q')
    {
            usleep(10000);
    }

    isp_stop = true;
    thread_isp.join();
    
    return 0;
}
