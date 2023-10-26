#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>


using namespace std;
int g_client_socket = 0;

bool is_file_updated(const std::string& filePath, time_t& lastModifiedTime)
{
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) != 0) {
        //std::cout << "Failed to get file information: " << filePath << std::endl;
        return false;
    }

    time_t modifiedTime = fileInfo.st_mtime;
    //cout << modifiedTime << endl;
    if (modifiedTime != lastModifiedTime) {
        lastModifiedTime = modifiedTime;
        return true;
    }

    return false;
}


bool is_tts_file_updated(const std::string& filePath, time_t& lastModifiedTime)
{
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) != 0) {
        //std::cout << "Failed to get file information: " << filePath << std::endl;
        return false;
    }

    time_t modifiedTime = fileInfo.st_mtime;
    //cout << modifiedTime << endl;
    if (modifiedTime != lastModifiedTime) {
        lastModifiedTime = modifiedTime;
        return true;
    }

    return false;
}


bool is_register_file_updated(const std::string& filePath, time_t& lastModifiedTime)
{
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) != 0) {
        //std::cout << "Failed to get file information: " << filePath << std::endl;
        return false;
    }

    time_t modifiedTime = fileInfo.st_mtime;
    //cout << modifiedTime << endl;
    if (modifiedTime != lastModifiedTime) {
        lastModifiedTime = modifiedTime;
        return true;
    }

    return false;
}


void send_file_to_server(const std::string &filePath) {

    std::ifstream jsonFile(filePath);
    if (!jsonFile) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << jsonFile.rdbuf();
    std::string jsonData = buffer.str();
    jsonFile.close();
    cout << "json content:" << jsonData << endl;
    int message_length = jsonData.size();
    char header_buf[4];
    *(int*)header_buf = htonl(message_length);
    send(g_client_socket, header_buf, sizeof(header_buf), 0);
    // cout << "数据大小发送成功" << endl;
    send(g_client_socket, jsonData.c_str(), message_length, 0);
    // cout << "数据发送成功" << endl;
}




int main(int argc, char* argv[]){

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <ip_address>" << std::endl;
        return 1;
    }


    g_client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_client_socket == -1) {
        cerr << "Failed to create socket." << endl;
        return 1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;

    serverAddr.sin_port = htons(std::atoi(argv[1]));
    inet_pton(AF_INET, argv[2], &serverAddr.sin_addr);

    if (connect(g_client_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Failed to connect." << endl;
        return 1;
    }
    std::string jsonFilePath = "dynamic_events.txt";
    time_t lastModifiedTime_dynamic = 0;
    std::string ttsFilePath = "tts_task.txt";
    time_t lastModifiedTime_tts = 0;
    std::string registerFilePath = "register_info.txt";
    time_t lastModifiedTime_register = 0;

    while (true){

        if (is_tts_file_updated(ttsFilePath, lastModifiedTime_tts)) {
            cout << "K230发送TTS文件给PC: "  << endl;
            send_file_to_server(ttsFilePath);
        }

        if (is_register_file_updated(registerFilePath, lastModifiedTime_register)) {
            cout << "K230发送注册文件给PC: "  << endl;
            send_file_to_server(registerFilePath);
        }

        if (is_file_updated(jsonFilePath, lastModifiedTime_dynamic)){
            cout << "K230发送动态json文件给chatgpt: "  << endl;
            send_file_to_server(jsonFilePath);


            cout << "chatgpt回复回传给K230: "  << endl;
            while (true){
                char header_buf[4];
                int bytes_received = 0;
                while (bytes_received < 4) {
                    bytes_received += recv(g_client_socket, header_buf + bytes_received, 4 - bytes_received, 0);
                }
                // char header_buf[256];
                // int bytes_received = 0;
                // recv(g_client_socket, header_buf + bytes_received, 4 - bytes_received, 0);
                int message_length = ntohl(*(int*)header_buf);
                // 根据消息体长度接收完整数据
                vector<char> reply_data;
                // cout<<"1 reply_data:"<<reply_data.size()<<endl;
                // cout<<"message_length:"<<message_length<<endl;
                reply_data.reserve(message_length);
                bytes_received = 0;
                // cout<<"2 reply_data:"<<reply_data.size()<<endl;

                while (bytes_received < message_length) {
                    char buf[1024];
                    int bytes = recv(g_client_socket, buf, min(1024, int(message_length - bytes_received)), 0);
                    if (bytes <= 0) {
                        break;
                    }
                    reply_data.insert(reply_data.end(), buf, buf + bytes);
                    bytes_received += bytes;
                }
                // cout<<"3 reply_data:"<<reply_data.size()<<endl;

                if (reply_data.empty()) {
                    break;
                }

                string reply_message(reply_data.begin(), reply_data.end());
                if (reply_message == "END") {
                    break;
                }
                else {
                    // cout << reply_message<<endl;
                    printf("%s",reply_message.c_str());
                }
            }
            cout << endl;
        }
        else{
            // cout << "没有新的json文件生成" << endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
    }
    close(g_client_socket);
    return 0;
}