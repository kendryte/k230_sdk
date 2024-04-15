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
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <termios.h>
#include <fstream>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <atomic>


using namespace std;

int g_client_socket = 0;

std::atomic<bool> g_running(true);


/**
 * @brief 信号处理函数，用于处理特定信号的操作。
 *
 * 在收到信号时，关闭套接字连接、退出程序。
 *
 * @param signum 信号编号
 */
void signal_handler(int signum)
{
    cout << "Closing socket connection..." << endl;
    close(g_client_socket);
    exit(signum);
}

/**
 * @brief SIGTSTP信号处理函数，用于处理Ctrl + Z信号的操作。
 *
 * 在收到信号时，关闭套接字连接、退出程序。
 *
 * @param signum 信号编号
 */
void sigtstp_handler(int signum)
{
    cout << "Closing socket connection..." << endl;
    close(g_client_socket);
    exit(signum);
}

/**
 * @brief 信号处理函数，用于处理Ctrl + '\'信号的操作。
 *
 * 当收到SIGQUIT信号时，将running标志设置为false。
 *
 * @param signum 信号编号
 */
void signalbreak_handler(int signum)
{
    if (signum == SIGQUIT) { // SIGQUIT is triggered by Ctrl + '\'
        g_running = false;
    }
}

/**
 * @brief 判断指定文件在sharefs下是否已更新。
 *
 * @param filePath 文件路径
 * @param lastModifiedTime 上次修改时间
 * @return 若文件已更新，则返回true；否则返回false
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
    while (g_running)
    {
        // 检查是否有按键按下，如果有按下 'q' 键，则设置退出标志
        if (is_key_pressed() == 'q')
        {
             g_running = false;
        }

        // 短暂休眠，避免过多消耗CPU资源
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return NULL;
}

/**
 * @brief 向服务器发送文件的内容。
 *
 * 打开指定文件，读取内容，并将内容发送到服务器。
 *
 * @param filePath 文件路径
 */
void send_file_to_server(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> txtData(fileSize);
    if (!file.read(txtData.data(), fileSize)) {
        std::cerr << "Failed to read file: " << filePath << std::endl;
        return;
    }
    file.close();

    // std::cout << "OCR识别结果:" << std::endl;
    std::cout.write(txtData.data(), fileSize);
    std::cout << std::endl;

    // 以'/txt'标识ocr识别结果
    std::string tag = "/txt";
    std::vector<char> message(tag.begin(), tag.end());
    message.insert(message.end(), txtData.begin(), txtData.end());

    // 编码信息长度，组成消息头
    uint32_t messageLength = htonl(message.size());
    std::vector<char> header(sizeof(messageLength));
    memcpy(header.data(), &messageLength, sizeof(messageLength));
    // 发送消息头+消息体
    send(g_client_socket, header.data(), header.size(), 0);
    send(g_client_socket, message.data(), message.size(), 0);

}



int main(int argc, char* argv[]) {
    
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <ip_address>" << std::endl;
        return 1;
    }

    std::ofstream clear_file("ocr.txt", std::ios::out | std::ios::trunc);
    clear_file.close();     
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_cc[VERASE] = 0x08; // ASCII code for Backspace
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    bool ocr_flag = false;
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

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGQUIT, signalbreak_handler);

    std::string ocrFilePath = "ocr.txt"; // Specify the path to your ocr.txt file.
    time_t imageLastModifiedTime = 0;
   
    // 获取初始的文件最后修改时间
    is_file_updated(ocrFilePath, imageLastModifiedTime);
    
    while (true) {

        if(ocr_flag){
            std::cout<<"已进入人机交互模式，请将手指放在摄像头可见区域内~"<<std::endl;
            while (g_running) {
                ocr_flag = false;
                if (is_file_updated(ocrFilePath, imageLastModifiedTime)) {

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    //发送ocr识别结果
                    send_file_to_server(ocrFilePath);
                    
                    g_running = false;
                }
            }
            g_running = true;

        }
    
        else{
            //发送消息
            string message;
            cout << "Enter your message: ";
            getline(cin, message);
            if (message.empty()) {
                break;
            }
            int message_length = message.size();
            char header_buf[4];
            *(int*)header_buf = htonl(message_length);
            send(g_client_socket, header_buf, sizeof(header_buf), 0);
            send(g_client_socket, message.c_str(), message_length, 0);

        }


        // 接收回传消息
        while (true) {
            // 先接收消息头，获取消息体的长度
            char header_buf[4];
            int bytes_received = 0;
            while (bytes_received < 4) {
                bytes_received += recv(g_client_socket, header_buf + bytes_received, 4 - bytes_received, 0);
            }
            int message_length = ntohl(*(int*)header_buf);

            // 根据消息体长度接收完整数据
            vector<char> reply_data;
            reply_data.reserve(message_length);
            bytes_received = 0;
            while (bytes_received < message_length) {
                char buf[1024];
                int bytes = recv(g_client_socket, buf, min(1024, message_length - bytes_received), 0);
                if (bytes <= 0) {
                    break;
                }
                reply_data.insert(reply_data.end(), buf, buf + bytes);
                bytes_received += bytes;
            }

            if (reply_data.empty()) {
                break;
            }

            string reply_message(reply_data.begin(), reply_data.end());
            if (reply_message.substr(0, 5) == "audio") {
                // 如果是音频数据，则保存音频
                std::ofstream outfile;
                outfile.open("audio.wav", std::ios::out | std::ios::binary);
                if (outfile.is_open()) {
                    outfile.write(reply_message.data()+5, reply_message.size()-5);
                    outfile.close();
                }
                break;
            }
            else if (reply_message.substr(0, 2) == "\xff\xd8") {

                // 如果是图片数据，则保存图片
                std::ofstream outfile;
                outfile.open("received_image.jpg", std::ios::out | std::ios::binary);
                if (outfile.is_open()) {
                    outfile.write(reply_message.data(), reply_message.size());
                    outfile.close();
                }
                break;
            }
            else if (reply_message.substr(0,5) == "/Task"){
                ocr_flag = true;
                std::ofstream outfile;
                outfile.open("param.txt", std::ios::out | std::ios::binary);
                if (outfile.is_open()) {
                    outfile.write(reply_message.data()+5, reply_message.size()-5);
                    outfile.close();
                }
                break;
            }
            if (reply_message == "END") {
                break;
            }
            else {
                cout << reply_message;
            }
        }
        cout << endl;
    }

    close(g_client_socket);

    return 0;
}