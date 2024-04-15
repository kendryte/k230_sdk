#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>


struct FaceResult {
    bool head_pose;
    std::string name;

    FaceResult() : head_pose(false), name("none") {}
};

struct TrackResult {
    std::string person_id;

    TrackResult() : person_id("none") {}
};

struct ActionResult {
    std::string action;

    ActionResult() : action("none") {}
};

//这些可以重复初始化
std::unordered_map<std::string, std::string> dynamic_events = {
    {"name" , ""},
    {"activity" , ""}
};

std::unordered_map<std::string, std::string> tts_task = {
    {"contents" , ""}
};

//这些应该放在启动摄像头之前，全局变量，不能重复初始化
bool Appearance_flag = false;
bool HeadPose_flag = false;
bool Face_flag = false;
bool Activity_flag = false;
bool Duration_flag = false;
int Disappearance_count = 0;
std::string name = "none";
int appearance_frame;
int head_pose_begin_frame;

//todo:这里应该将各个AI Demo的检测结果填入上述三个struct中，下面代码将基于上述三个struct进行信息的提取。
TrackResult track;
FaceResult face;
ActionResult action;
//todo:这里对接帧数。
int frame = 0;

void extract_json_or_task(void) {
    //但凡有人出现就得置零。
    Disappearance_count = 0;
    //先判断有人出现，从track入手，当track不是空的时候且Appearence_flag == false时证明人物第一次出现了
    if (track.person_id != "none") {

        //任务一：第一次有人出现时，将Appearance_flag设为true，为后面的消失判断做准备。
        if (Appearance_flag == false) {
            Appearance_flag = true;
            appearance_frame = frame;
            tts_task["contents"] = "您好，请面向摄像头以辨认您的身份。";

            //保存task为本地文件
            std::ofstream outFile("tts_task.txt");
            if (outFile.is_open()) {
                for (const auto& entry : tts_task) {
                    outFile << entry.first << " " << entry.second << "\n";
                }
                outFile.close();
            } else {
                std::cout << "Unable to open the tts_task.txt file for writing." << std::endl;
            }

            //这100帧是从当前帧到用户接收到信息作出反应的时间。
            head_pose_begin_frame = frame + 100;

            //防止下方保存动作覆盖当前保存动作
            return;
        }

        //任务二：判断头部姿态角是否符合要求。
        if (HeadPose_flag == false && frame > head_pose_begin_frame) {
            if (face.head_pose == true && face.name != "none") {
                HeadPose_flag = true;
                name = face.name;
                tts_task["contents"] = face.name + "谢谢您的配合！";
                std::ofstream outFile("tts_task.txt");
                if (outFile.is_open()) {
                    for (const auto& entry : tts_task) {
                        outFile << entry.first << " " << entry.second << "\n";
                    }
                    outFile.close();
                } else {
                    std::cout << "Unable to open the tts_task.txt file for writing." << std::endl;
                }
                return;
                }
            else {
                tts_task["contents"] = "请正视摄像头！";
                std::ofstream outFile("tts_task.txt");
                if (outFile.is_open()) {
                    for (const auto& entry : tts_task) {
                        outFile << entry.first << " " << entry.second << "\n";
                    }
                    outFile.close();
                } else {
                    std::cout << "Unable to open the tts_task.txt file for writing." << std::endl;
                }
                return;
            }
        }

        //任务三：识别到人脸后生成动态JSON。
        if (Face_flag == false) {
            Face_flag = true;
            dynamic_events["name"] = name;
            dynamic_events["activity"] = "Just walk home.";
            std::ofstream outFile("dynamic_events.txt");
            if (outFile.is_open()) {
                for (const auto& entry : dynamic_events) {
                    outFile << entry.first << " " << entry.second << "\n";
                }
                outFile.close();
            } else {
                std::cout << "Unable to open the dynamic_events.txt file for writing." << std::endl;
            }
            return;
        }

        //任务四：摔倒生成动态JSON。
        if (Activity_flag == false && action.action == "down") {
            Activity_flag = true;
            dynamic_events["name"] = name;
            dynamic_events["activity"] = "Fall down.";
            std::ofstream outFile("dynamic_events.txt");
            if (outFile.is_open()) {
                for (const auto& entry : dynamic_events) {
                    outFile << entry.first << " " << entry.second << "\n";
                }
                outFile.close();
            } else {
                std::cout << "Unable to open the dynamic_events.txt file for writing." << std::endl;
            }
            return;
        }

        //任务五：长时间逗留生成动态JSON。
        if (Duration_flag == false && frame - appearance_frame > 300) {
            Duration_flag = true;
            dynamic_events["name"] = name;
            dynamic_events["activity"] = "Has been here for a long time.";
            std::ofstream outFile("dynamic_events.txt");
            if (outFile.is_open()) {
                for (const auto& entry : dynamic_events) {
                    outFile << entry.first << " " << entry.second << "\n";
                }
                outFile.close();
            } else {
                std::cout << "Unable to open the dynamic_events.txt file for writing." << std::endl;
            }
            return;
        }
    } else {
        Disappearance_count = Disappearance_count + 1;
        if (Appearance_flag == true && Disappearance_count > 10) {
            Appearance_flag = false;
            dynamic_events["name"] = name;
            dynamic_events["activity"] = "Leave home.";
            std::ofstream outFile("dynamic_events.txt");
            if (outFile.is_open()) {
                for (const auto& entry : dynamic_events) {
                    outFile << entry.first << " " << entry.second << "\n";
                }
                outFile.close();
            } else {
                std::cout << "Unable to open the dynamic_events.txt file for writing." << std::endl;
            }
            return;
        }
    }
}
