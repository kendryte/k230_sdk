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

using namespace std;

int client_socket = 0;

void signalHandler(int signum)
{
    cout << "Closing socket connection..." << endl;
    close(client_socket);
    exit(signum);
}

void sigtstpHandler(int signum)
{
    cout << "Closing socket connection..." << endl;
    close(client_socket);
    exit(signum);
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <ip_address>" << std::endl;
        return 1;
    }
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_cc[VERASE] = 0x08; // ASCII code for Backspace
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == -1) {
        cerr << "Failed to create socket." << endl;
        return 1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(std::atoi(argv[1]));
    inet_pton(AF_INET, argv[2], &serverAddr.sin_addr);
    if (connect(client_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Failed to connect." << endl;
        return 1;
    }

    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGTSTP, sigtstpHandler);

    while (true) {
        // 发送消息
        string message;
        cout << "Enter your message: ";
        getline(cin, message);
        if (message.empty()) {
            break;
        }

        message = message + "\r\n";

        int message_length = message.size();
        char header_buf[4];
        *(int*)header_buf = htonl(message_length);
        send(client_socket, header_buf, sizeof(header_buf), 0);
        send(client_socket, message.c_str(), message_length, 0);

        // 接收回传消息
        while (true) {
            // 先接收消息头，获取消息体的长度
            char header_buf[4];
            int bytes_received = 0;
            while (bytes_received < 4) {
                bytes_received += recv(client_socket, header_buf + bytes_received, 4 - bytes_received, 0);
            }
            int message_length = ntohl(*(int*)header_buf);
            // 根据消息体长度接收完整数据
            vector<char> reply_data;
            reply_data.reserve(message_length);
            bytes_received = 0;
            while (bytes_received < message_length) {
                char buf[1024];
                int bytes = recv(client_socket, buf, min(1024, message_length - bytes_received), 0);
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
                    outfile.write(reply_message.data()+5, reply_message.size()-5); // Skip first 5 characters, because they are the "audio" tag
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
            if (reply_message == "END") {
                break;
            }
            else {
                cout << reply_message;
            }
        }
        cout << endl;
    }

    close(client_socket);

    return 0;
}
