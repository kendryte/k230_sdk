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
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <dirent.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

const int BUFFER_SIZE = 4096;
const int COMMAND_SIZE = 1024;

// 新的函数用于发送响应消息
bool sendResponse(int socket, const char* response) {
    ssize_t bytes_sent = send(socket, response, strlen(response), 0);
    if (bytes_sent == -1) {
        cerr << "发送响应失败" << endl;
        return false;
    }
    return true;
}

bool isDirectory(const char* path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        return false;
    }
    return S_ISDIR(path_stat.st_mode);
}

bool deleteDirectory(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char entryPath[PATH_MAX];
            snprintf(entryPath, sizeof(entryPath), "%s/%s", path, entry->d_name);

            if (isDirectory(entryPath)) {
                deleteDirectory(entryPath);
            } else {
                std::remove(entryPath);
            }
        }
    }
    closedir(dir);
    std::remove(path);
    return true;
}


string readResult(const string& file_path) {
    string result = "";
    ifstream file(file_path);
    
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            result += line + '\n';
        }
        file.close();
    } else {
        cerr << "无法打开文件：" << file_path << endl;
    }
    
    return result;
}

int sendFileToServer(int client_socket, const char* file_path) {
    FILE* image_file = fopen(file_path, "rb");
    if (image_file == nullptr) {
        perror("无法打开图片文件");
        return 2;
    }

    fseek(image_file, 0, SEEK_END);
    long image_size = ftell(image_file);
    fseek(image_file, 0, SEEK_SET);

    if (send(client_socket, &image_size, sizeof(image_size), 0) == -1) {
        cerr << "Error sending image size to server" << endl;
        fclose(image_file);
        return -1;
    }

    char buffer[BUFFER_SIZE];
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

    fclose(image_file);
    if (image_size <= 0) {
        return 0;
    } else {
        return 5;
    }
}

void handleCommand(int client_socket, const char* command) {
    if (strcmp(command, "on") == 0) {
        ofstream file("flag/weight.txt");
        if (file.is_open()) {
            file.close();
            cout << "已创建文件 flag/weight.txt" << endl;
        }
        sendResponse(client_socket, "image");
        int count=0;
        while (access("res/res.jpg", F_OK) == -1) {
            this_thread::sleep_for(chrono::seconds(1));
            count++;
            if(count>200){
                break;
            }
        }

        int res = sendFileToServer(client_socket, "res/res.jpg");
        if (res == 0) {
            cout << "image图片发送成功" << endl;
        }
    } else if (strcmp(command, "get") == 0) {
        sendResponse(client_socket, "result");

        while (access("res/result.txt", F_OK) == -1) {
            this_thread::sleep_for(chrono::seconds(1));
        }

        string result = readResult("res/result.txt");
        if (!result.empty()) {
            sendResponse(client_socket, result.c_str());
        }
    } else if (strcmp(command, "select") == 0) {
        ofstream file("flag/select.txt");
        if (file.is_open()) {
            file.close();
            cout << "已创建文件 flag/select.txt" << endl;
        }
        sendResponse(client_socket, "select");

        int count=0;
        while (access("res/select.jpg", F_OK) == -1) {
            this_thread::sleep_for(chrono::seconds(1));
            count++;
            if(count>200){
                break;
            }
        }

        int res = sendFileToServer(client_socket, "res/select.jpg");
        if (res == 0) {
            cout << "select图片发送成功" << endl;
            remove("./flag/select.txt");
            remove("./res/select.jpg");
        }
    } else if (strcmp(command, "import") == 0) {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytesRead < 0) {
            perror("Error in receiving data");
            exit(EXIT_FAILURE);
        }

        string receivedString(buffer);
        ofstream outputFile("flag/goodinf.txt");
        if (!outputFile) {
            perror("Error in opening goodinf.txt");
            exit(EXIT_FAILURE);
        }
        outputFile << receivedString;
        outputFile.close();

        ofstream file("flag/import.txt");
        if (file.is_open()) {
            file.close();
            cout << "已创建文件 flag/import.txt" << endl;
            sendResponse(client_socket, "import");
        }
    }else if (strcmp(command, "clear") == 0) {
        ofstream file("flag/clear.txt");
        if (file.is_open()) {
            file.close();
            cout << "已创建文件 flag/clear.txt" << endl;
            
        }

        while (access("flag/clear_c.txt", F_OK) == -1) {
            this_thread::sleep_for(chrono::seconds(1));
        }
        string folderpath=readResult("flag/clear_c.txt");
        if(deleteDirectory(folderpath.c_str())){
            remove("falg/clear.txt");
            remove("flag/clear_c.txt");
        }
        else{

        }
        sendResponse(client_socket, "clear");
    } else if (strcmp(command, "down") == 0) {
        if (remove("flag/weight.txt") == 0) {
            cout << "已删除文件 flag/weight.txt" << endl;
        } else {
            cerr << "无法删除文件" << endl;
        }
        if (remove("res/res.jpg") == 0) {
            cout << "成功删除 res/res.jpg 文件" << endl;
        } else {
            perror("无法删除 res/res.jpg 文件");
        }
        if (remove("res/result.txt") == 0) {
            cout << "成功删除 res/result.txt 文件" << endl;
            sendResponse(client_socket, "stop");
        } else {
            perror("无法删除 res/result.txt 文件");
        }
    } else if (strcmp(command, "close") == 0) {
        close(client_socket);
        exit(0);
    } else {
        cerr << "未知命令: " << command << endl;
        sendResponse(client_socket, "unknown");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return EXIT_FAILURE;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("无法创建套接字");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &(server_address.sin_addr)) <= 0) {
                perror("无法设置服务器地址");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("连接到服务器失败");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    char command[COMMAND_SIZE];
    while (1) {
        ssize_t bytes_received = recv(client_socket, command, sizeof(command), 0);
        if (bytes_received <= 0) {
            // 处理接收错误
            cerr << "与服务器的连接已断开" << endl;
            close(client_socket);
            exit(EXIT_FAILURE);
        } else {
            command[bytes_received] = '\0';
            cout << "收到命令：" << command << endl;
            if (strcmp(command, "close") == 0) {
                close(client_socket);
                exit(0);
            }
            handleCommand(client_socket, command);
        }
    }

    close(client_socket);

    return 0;
}

       
