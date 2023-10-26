#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <thread>

using namespace std;

// 新的函数用于发送响应消息
bool sendResponse(int socket, const char* response) {
    ssize_t bytes_sent = send(socket, response, strlen(response), 0);
    if (bytes_sent == -1) {
        std::cerr << "发送响应失败" << std::endl;
        return false;
    }
    return true;
}

std::string read_result(const std::string& file_path) {
    std::string result="";
    std::ifstream file(file_path);
    
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            result += line + '\n';
        }
        file.close();
    } else {
        std::cerr << "无法打开文件：" << file_path << std::endl;
    }
    
    return result;
}

int sendImagesToServer(int client_socket) {
    // 打开 res.jpg 文件
    FILE* image_file = fopen("res/res.png", "rb");
    if (image_file == NULL) {
        perror("无法打开图片文件");
        return 2;
    }
    // 获取文件大小
    fseek(image_file, 0, SEEK_END);
    long image_size = ftell(image_file);
    fseek(image_file, 0, SEEK_SET);
    // 将文件大小发送到服务器
    if (send(client_socket, &image_size, sizeof(image_size), 0) == -1) {
        std::cerr << "Error sending image size to server" << std::endl;
        // close(client_socket);
        return -1;
    }
    // 读取文件内容并发送给服务器
    // vector<char> image_data = read_binary_file<char>(image_file);
    char buffer[4096];
    while (image_size > 0) {
        size_t read_size = fread(buffer, 1, sizeof(buffer), image_file);
        if (read_size <= 0) {
            perror("读取文件失败");
            fclose(image_file);
            return 3;
        }

        ssize_t sent_size = send(client_socket, buffer, read_size, 0);
        if (sent_size == -1) {
            perror("发送文件失败");
            fclose(image_file);
            return 4;
        }
        image_size -= sent_size;
    }
    // 关闭文件
    fclose(image_file);
    if (image_size<=0){
        return 0;
    }else{
        return 5;
    }
}


void handleCommand(int client_socket, const char* command) {
    if (strcmp(command, "on") == 0) {
        // 创建一个文件
        std::ofstream file("flag/weight.txt");
        if (file.is_open()) {
            file.close();
        }
        sendResponse(client_socket,"image");
        int count=0;
        while(1){
            if((access("res/res.png", F_OK) != -1)){
                int res=sendImagesToServer(client_socket);
                if(res==0){
                    break;
                }else{
                    continue;
                }    
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    }else if(strcmp(command, "get") == 0){
        sendResponse(client_socket,"result");
        while(1){
            if((access("res/result.txt", F_OK) != -1)){
                string result=read_result("res/result.txt");
                if(result==""){

                }else{
                    sendResponse(client_socket,result.c_str());
                    break;
                }
                
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    }else if (strcmp(command, "down") == 0) {
        // 删除文件
        if (remove("flag/weight.txt") == 0) {
            // std::cout << "已删除文件 flag/weight.txt" << std::endl;
        } else {
            std::cerr << "无法删除文件" << std::endl;
        }
        if ((remove("res/res.png") == 0)) {
            // std::cout << "成功删除 res/res.png文件" << std::endl;
        } else {
            perror("无法删除 res/res.png文件");
        }
        if ((remove("res/result.txt") == 0)) {
            // std::cout << "成功删除 res/result.txt文件" << std::endl;
            sendResponse(client_socket, "stop");
        } else {
            perror("无法删除 res/result.txt文件");
        }
    }else if(strcmp(command, "close") == 0){
        close(client_socket);
        exit(0);

    }else {
        std::cerr << "未知命令: " << command << std::endl;
        sendResponse(client_socket, "unknown"); // 发送未知命令消息
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* server_ip = argv[1];// 服务器的IP地址
    int server_port = std::atoi(argv[2]);// 服务器的端口号

    // 创建套接字
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("无法创建套接字");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址信息
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &(server_address.sin_addr)) <= 0) {
        perror("无法设置服务器地址");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // 连接到服务器
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("连接到服务器失败");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // 连接成功，接收服务器命令
    char command[1024];
    while (1) {
        ssize_t bytes_received = recv(client_socket, command, sizeof(command), 0);
        if (bytes_received <= 0) {
            
        } else {
            command[bytes_received] = '\0';
            // std::cout << "收到命令：" << command << std::endl;
            if(strcmp(command, "close") == 0){
                close(client_socket);
                exit(0);
            }
            handleCommand(client_socket,command);
        }
    }

    // 关闭套接字
    close(client_socket);

    return 0;
}
