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
#include <thread>
#include <map>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "utils.h"
#include "vi_vo.h"
#include "gen_embedding.h"

using std::cerr;
using std::cout;
using std::endl;
using namespace std;

std::atomic<bool> isp_stop(false);

std::vector<std::vector<float>> g_embeddings; //底库数据创建的向量对比库
std::map<size_t, std::string> g_label_dict; // 底库类别标签映射
std::map<int,float> g_price_dict; // 底库类别单价映射
int g_img_idx=0; // 底库图片索引
int g_max_classes_idx=0; // 底库类别索引
string g_dataset_dir="gallery"; // 底库根目录
string g_result=""; // 识别类别结果
string g_price="";  // 识别类别单价
string g_weight=""; // 识别类别重量，默认为1kg，实际应用中可以接入重量传感器

void print_usage()
{
    cout << "模型推理时传参说明："
         << "<kmodel_path> <image_path> <dataset_dir> <top_k> <debug_mode>" << endl
         << "Options:" << endl
         << "  kmodel_path     Kmodel的路径\n"
         << "  image_path      待推理图片路径/摄像头(None)\n"
         << "  dataset_dir     创建向量库使用类别图片数据路径,目录结构如下：\n"
         << "                  |-dataset_dir\n"
         << "                      |-class1\n"
         << "                          |-pic1.jpg\n"
         << "                          |-pic2.jpg\n"
         << "                          |-...\n"
         << "                          |-label.txt\n"
         << "                      |-class2\n"
         << "                          |-...\n"
         << "                      |-...\n"
         << "  top_k           度量top_k参数\n"
         << "  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}

bool file_exists(const std::string &filename) {
    std::ifstream file(filename.c_str());
    return file.good(); 
}

bool removeFile(const std::string& filePath) {
    if (remove(filePath.c_str()) == 0) {
        return true;
    }
    return false;
}

bool clearDirectory(const std::string& path) {
    if (mkdir(path.c_str(), 0777) != 0) {
        return false;
    }

    DIR* dp = opendir(path.c_str());
    if (!dp) {
        return false;
    }

    struct dirent* entry;

    while ((entry = readdir(dp))) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            std::string entryPath = path + "/" + entry->d_name;
            struct stat statbuf;

            if (stat(entryPath.c_str(), &statbuf) == -1) {
                return false;
            }

            if (S_ISDIR(statbuf.st_mode)) {
                // 递归删除子文件夹
                if (!clearDirectory(entryPath)) {
                    return false;
                }
            } else {
                // 删除文件
                if (!removeFile(entryPath)) {
                    return false;
                }
            }
        }
    }
    closedir(dp);

    return true;
}

bool directoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false; // 文件夹不存在
    }

    return (info.st_mode & S_IFDIR) != 0; // 文件夹存在
}

bool createDirectory(const std::string& path) {
    if (mkdir(path.c_str(), 0777) == 0) {
        return true; // 文件夹创建成功
    }

    return false; // 文件夹创建失败
}


bool copyFile(const std::string &sourceFilePath,const std::string &destinationFilePath){
    std::ifstream sourceFile(sourceFilePath, std::ios::binary);
    if (!sourceFile) {
        std::cerr << "无法打开源文件: " << sourceFilePath << std::endl;
        return 1;
    }
    std::ofstream destinationFile(destinationFilePath, std::ios::binary);
    if (!destinationFile) {
        std::cerr << "无法创建或打开目标文件: " << destinationFilePath << std::endl;
        return 1;
    }
    // 逐个字节复制文件内容
    char buffer;
    while (sourceFile.get(buffer)) {
        destinationFile.put(buffer);
    }
    // 关闭文件
    sourceFile.close();
    destinationFile.close();

    std::cout << "文件拷贝成功!" << std::endl;
}

bool copytxtFile(const std::string &sourceFilePath,const std::string &destinationFilePath){
    std::ifstream sourceFile(sourceFilePath); // 打开源文件
    if (!sourceFile.is_open()) {
        std::cerr << "无法打开源文件" << std::endl;
        return false;
    }

    std::ofstream destinationFile(destinationFilePath); // 创建或打开目标文件
    if (!destinationFile.is_open()) {
        std::cerr << "无法创建或打开目标文件" << std::endl;
        sourceFile.close(); // 关闭源文件
        return false;
    }

    std::string line;
    while (std::getline(sourceFile, line)) {
        destinationFile << line << std::endl; // 逐行复制内容
    }

    sourceFile.close(); // 关闭源文件
    destinationFile.close(); // 关闭目标文件

    return true;

}

int save_result(string file_name,string recog_res)
{
    std::ofstream outfile(file_name);
    if (!outfile)
    {
        std::cout << "无法打开文件"+file_name << std::endl;
        return 1;
    }
    outfile << recog_res;
    outfile.close();
    return 0;
}

double calculate_cosine_similarity(const std::vector<float> &embedding1, const std::vector<float> &embedding2)
{
    double dot_product = 0.0;
    double norm_embedding1 = 0.0;
    double norm_embedding2 = 0.0;

    for (size_t i = 0; i < 512; ++i)
    {
        dot_product += embedding1[i] * embedding2[i];
        norm_embedding1 += embedding1[i] * embedding1[i];
        norm_embedding2 += embedding2[i] * embedding2[i];
    }

    if (norm_embedding1 == 0.0 || norm_embedding2 == 0.0)
    {
        return 0.0; // To avoid division by zero
    }

    return dot_product / (std::sqrt(norm_embedding1) * std::sqrt(norm_embedding2));
}

std::vector<std::pair<size_t, double>> calculate_cosine_similarity_with_all_files(const std::vector<float> &target_embedding)
{
    ScopedTiming st("calculate cosine similarity with all files", 2);
    std::vector<std::pair<size_t, double>> similarities;
    for (size_t i = 0; i < g_embeddings.size(); ++i)
    {
        double similarity = calculate_cosine_similarity(target_embedding, g_embeddings[i]);
        similarities.push_back(std::make_pair(i, similarity));
    }
    return similarities;
}

std::pair<std::vector<size_t>, std::string> get_top_n_similar_files(const std::vector<float> &target_embedding, size_t n, int debug_mode)
{
    ScopedTiming st("get top_k idx", debug_mode);
    std::vector<std::pair<size_t, double>> similarities = calculate_cosine_similarity_with_all_files(target_embedding);
    std::sort(similarities.begin(), similarities.end(), [](const auto &a, const auto &b)
              { return a.second > b.second; });
    std::vector<size_t> top_n_indices;
    for (size_t i = 0; i < n && i < similarities.size(); ++i)
    {
        top_n_indices.push_back(similarities[i].first);
    }

    std::vector<std::string> top_labels;
    for (const auto &index : top_n_indices)
    {
        top_labels.push_back(g_label_dict.at(index));
    }
    std::map<std::string, int> element_counts;
    for (const auto &label : top_labels)
    {
        element_counts[label]++;
    }
    std::string most_common_element = std::max_element(
                                          element_counts.begin(), element_counts.end(),
                                          [](const auto &a, const auto &b)
                                          { return a.second < b.second; })
                                          ->first;
    return std::make_pair(top_n_indices, most_common_element);
}

vector<float> image_proc(string &kmodel_path, string &image_path, int debug_mode)
{
    ScopedTiming st("generate embedding by image", debug_mode);
    cv::Mat ori_img = cv::imread(image_path);
    int ori_w = ori_img.cols;
    int ori_h = ori_img.rows;
    GenEmbedding ge(kmodel_path, debug_mode);
    ge.pre_process(ori_img);
    ge.inference();
    std::vector<float> result;
    ge.post_process(result);
    return result;
}

int build_gallery(std::string &kmodel_path,  std::string &dataset_dir, int debug_mode) {
    ScopedTiming st("build gallery", debug_mode);
    GenEmbedding ge(kmodel_path, 0);

    // Open the directory using traditional method
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(dataset_dir.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            std::string category_dir_name = ent->d_name;
            if (category_dir_name == "." || category_dir_name == "..") {
                continue;
            }
            std::string category_dir_path = dataset_dir + "/" + category_dir_name;

            // Read the single label from labels.txt in each subdirectory
            std::string labels_file_path = category_dir_path + "/label.txt";
            if(!file_exists(labels_file_path)){
                continue;
            }
            std::ifstream labels_file(labels_file_path);
            if (!labels_file.is_open()) {
                std::cerr << "Error: Could not open labels file for directory " << category_dir_path << std::endl;
                continue;
            }

            std::string label_name;
            std::string shop_price;
            if (std::getline(labels_file, label_name)&&std::getline(labels_file,shop_price)) {
                // Open and process images in the category directory
                DIR *image_dir;
                struct dirent *img_ent;
                if ((image_dir = opendir(category_dir_path.c_str())) != nullptr) {
                    while ((img_ent = readdir(image_dir)) != nullptr) {
                        std::string image_name = img_ent->d_name;
                        if (image_name == "." || image_name == ".." || image_name.substr(image_name.find_last_of(".") + 1) != "jpg") {
                            continue;
                        }
                        std::string image_path = category_dir_path + "/" + image_name;
                        if(!file_exists(image_path)){
                            continue;
                        }
                        cv::Mat ori_img = cv::imread(image_path);
                        int ori_w = ori_img.cols;
                        int ori_h = ori_img.rows;
                        ge.pre_process(ori_img);
                        ge.inference();
                        std::vector<float> result;
                        ge.post_process(result);
                        g_embeddings.push_back(result);
                        // Use the single label for all images in this category
                        g_label_dict[g_img_idx] = label_name;
                        g_price_dict[g_img_idx] = std::stof(shop_price);
                        g_img_idx++;
                    }
                    closedir(image_dir);
                }
            }
            labels_file.close();
            g_max_classes_idx++;
        }
        closedir(dir);
    } else {
        std::cerr << "Error: Could not open directory " << dataset_dir << std::endl;
    }

    return 0;
}


void video_proc(string &kmodel_path, size_t top_k, int debug_mode)
{
    
    vivcap_start();

    k_video_frame_info vf_info;
    void *pic_vaddr = NULL; // osd

    memset(&vf_info, 0, sizeof(vf_info));

    vf_info.v_frame.width = osd_width;
    vf_info.v_frame.height = osd_height;
    vf_info.v_frame.stride[0] = osd_width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    block = vo_insert_frame(&vf_info, &pic_vaddr);

    // alloc memory
    size_t paddr = 0;
    void *vaddr = nullptr;
    size_t size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", size);
    if (ret)
    {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

    GenEmbedding ge(kmodel_path,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), debug_mode);

    vector<float> result;
    string weight_flag="flag/weight.txt";
    string select_flag="flag/select.txt";
    string import_flag="flag/import.txt";
    string clear_flag="flag/clear.txt";
    bool rec_flag = false; 
    bool slc_flag=false;
    bool add_class= false;
    bool clr_flag=false;
    int select_img_idx=0;
    string new_class_floder_name="";

    while (!isp_stop)
    {
        ScopedTiming st("total time", debug_mode);

        {
            ScopedTiming st("read capture", debug_mode);
            // VICAP_CHN_ID_1 out rgb888p
            memset(&dump_info, 0, sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret)
            {
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                continue;
            }
        }

        {
            ScopedTiming st("isp copy", debug_mode);
            // 从vivcap中读取一帧图像到dump_info
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3); // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        //如果采集标识文件存在，开始采集图片
        if(file_exists(select_flag)){
            if(slc_flag){

            }else{
                Utils::dump_color_image("res/select.jpg",{SENSOR_WIDTH,SENSOR_HEIGHT}, reinterpret_cast<unsigned char *>(vaddr));
                std::ostringstream oss_s;
                oss_s << select_img_idx;
                std::string img_idx = oss_s.str();
                select_img_idx++;
                if(!add_class){
                    std::ostringstream oss_c;
                    oss_c << g_max_classes_idx;
                    std::string class_idx = oss_c.str();
                    new_class_floder_name=g_dataset_dir+"/"+class_idx;
                    if (directoryExists(new_class_floder_name)) {
                        std::cout << "文件夹已存在" << std::endl;
                    } else {
                        if (createDirectory(new_class_floder_name)) {
                            std::cout << "文件夹创建成功" << std::endl;
                            add_class=true;
                            g_max_classes_idx++;
                        } else {
                            std::cerr << "无法创建文件夹" << std::endl;
                        }
                    }
                }else{
                    string source_path="res/select.jpg";
                    string destination_path=new_class_floder_name+"/"+img_idx+".jpg";
                    copyFile(source_path,destination_path);
                }
                slc_flag=true;
            }
        }else{
            if(slc_flag){
                slc_flag=false;
            }else{

            }
        }

        //如果导入标识文件存在，开始添加新类别到embeddings
        if(file_exists(import_flag)){
            if(file_exists("flag/goodinf.txt"))
            {
                string source_path="flag/goodinf.txt";
                string destination_path=new_class_floder_name+"/label.txt";
                copytxtFile(source_path,destination_path);
                {
                    // Open the directory using traditional method
                    DIR *dir;
                    struct dirent *ent;
                    std::string category_dir_path = new_class_floder_name;
                    // Read the single label from labels.txt in each subdirectory
                    std::string labels_file_path = category_dir_path + "/label.txt";
                    std::ifstream labels_file(labels_file_path);
                    if (!labels_file.is_open()) {
                        std::cerr << "Error: Could not open labels file for directory " << category_dir_path << std::endl;
                        continue;
                    }

                    std::string label_name;
                    std::string shop_price;
                    if (std::getline(labels_file, label_name)&&std::getline(labels_file,shop_price)) {
                        // Open and process images in the category directory
                        DIR *image_dir;
                        struct dirent *img_ent;
                        if ((image_dir = opendir(category_dir_path.c_str())) != nullptr) {
                            while ((img_ent = readdir(image_dir)) != nullptr) {
                                std::string image_name = img_ent->d_name;
                                if (image_name == "." || image_name == ".." || image_name.substr(image_name.find_last_of(".") + 1) != "jpg") {
                                    continue;
                                }
                                std::string image_path = category_dir_path + "/" + image_name;
                                cv::Mat ori_img = cv::imread(image_path);
                                int ori_w = ori_img.cols;
                                int ori_h = ori_img.rows;
                                ge.pre_process(ori_img);
                                ge.inference();
                                std::vector<float> result;
                                ge.post_process(result);
                                g_embeddings.push_back(result);
                                // Use the single label for all images in this category
                                g_label_dict[g_img_idx] = label_name;
                                g_price_dict[g_img_idx] = std::stof(shop_price);
                                g_img_idx++;
                                
                            }
                            closedir(image_dir);
                        }
                    }
                    labels_file.close();
                }
                std::remove("flag/goodinf.txt");
                std::remove(import_flag.c_str());
                select_img_idx=0;
                add_class=false;
                new_class_floder_name="";
            
            }

        }

        if(file_exists(clear_flag)){
            if(clr_flag){

            }else{
                std::ostringstream oss_c;
                oss_c << (g_max_classes_idx-1);
                std::string class_idx = oss_c.str();
                string newest_class_path=g_dataset_dir+"/"+class_idx;
                cout<<newest_class_path<<endl;
                save_result("flag/clear_c.txt",newest_class_path);
                select_img_idx=0;
                g_max_classes_idx--;
                clr_flag=true;
            }
            
        }else{
            if(clr_flag){
                clr_flag=false;
            }else{

            }
        }

        //如果称重标识存在，开始称重
        if (file_exists(weight_flag)){
            if (rec_flag){
            }
            else
            {
                Utils::dump_color_image("res/res.jpg",{SENSOR_WIDTH,SENSOR_HEIGHT}, reinterpret_cast<unsigned char *>(vaddr));
                result.clear();
                ge.pre_process();
                ge.inference();
                ge.post_process(result);
                rec_flag=true;
            }

        }
        else{
            if (rec_flag){
                rec_flag=false;
            }
            else
            {

            }
        }

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::Mat osd_frame_tmp;

        if(file_exists(weight_flag) && rec_flag){
            ScopedTiming st("embedding search", debug_mode);
            // Get the top k similar files
            std::pair<std::vector<size_t>, std::string> indexis = get_top_n_similar_files(result, top_k, debug_mode);
            // Print the result
            std::cout << "Top " << top_k << " similar file indices:[";
            for (const auto &idx : indexis.first)
            {
                std::cout << " " << idx;
            }
            std::cout << "] Recognition result: " << indexis.second << std::endl;
            g_result = indexis.second;
            g_weight=std::to_string(1);
            g_price=std::to_string(1*g_price_dict.at(indexis.first[0]));
            string text=g_result+"\n"+g_weight+"\n"+g_price;
            save_result("res/result.txt",text);
        }

        {
            ScopedTiming st("osd copy", debug_mode);
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            // 显示通道插入帧
            kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); // K_VO_OSD0
            printf("kd_mpi_vo_chn_insert_frame success \n");

            ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
            if (ret)
            {
                printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
            }
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

void image_recognition(string &kmodel_path, string &image_path, string &dataset_dir, int top_k, int debug_mode)
{
    build_gallery(kmodel_path,dataset_dir,debug_mode);
    std::vector<float> target_embedding = image_proc(kmodel_path, image_path, debug_mode);
    // Get the top k similar files
    std::pair<std::vector<size_t>, std::string> result = get_top_n_similar_files(target_embedding, top_k, debug_mode);
    // Print the result
    std::cout << "Top " << top_k << " similar file indices:";
    for (const auto &index : result.first)
    {
        std::cout << " " << index;
    }
    std::cout << " Recognition result: " << result.second << std::endl;
}

void video_recognition(char *argv[])
{
    string kmodel_path = argv[1];
    string image_path = argv[2];
    string dataset_dir=argv[3];
    size_t top_k = std::stoi(argv[4]);
    int debug_mode = std::stoi(argv[5]);
    build_gallery(kmodel_path,dataset_dir,debug_mode);
    video_proc(kmodel_path, top_k, debug_mode);
}

int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc < 6)
    {
        print_usage();
        return -1;
    }
    string kmodel_path = argv[1];
    string image_path = argv[2];
    string dataset_dir=argv[3];
    size_t top_k = std::stoi(argv[4]);
    int debug_mode = std::stoi(argv[5]);
    g_dataset_dir=dataset_dir;
    // video
    if (strcmp(argv[2], "None") == 0)
    {
        std::thread thread_isp(video_recognition, argv);
        while (getchar() != 'q')
        {
            usleep(10000);
        }

        isp_stop = true;
        thread_isp.join();
    }
    // image
    else
    {
        image_recognition(kmodel_path, image_path, dataset_dir, top_k, debug_mode);
    }
    return 0;
}
