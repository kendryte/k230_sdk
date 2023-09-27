#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>
#include <sys/stat.h>
#include<cstdio>
#include "scoped_timing.hpp"

using namespace std;

std::string readBinInt(std::string path)
{
    ifstream file2(path, std::ios::binary);
    int len2 = 0;
    file2.read((char*)&len2, sizeof(len2));
    char* buffer = new char[len2 + 1];
    file2.read(buffer, len2);
    buffer[len2] = '\0';
    string str2 = buffer;
    delete[] buffer;
    file2.close();

    return str2;
}

std::vector<float> read_binary_file(const char* file_name)
{
	std::ifstream ifs(file_name, std::ios::binary);
	ifs.seekg(0, ifs.end);
	size_t len = ifs.tellg();
    std::vector<float> vec(len / sizeof(float), 0);
	ifs.seekg(0, ifs.beg);
	ifs.read(reinterpret_cast<char*>(vec.data()), len);
	ifs.close();
	return vec;
}

bool Is_File_Exist(const std::string& file_path)
{
    std::ifstream file(file_path.c_str());
    return file.good();
}

bool isFolderExist(string folderPath) {
    struct stat info;
    if (stat(folderPath.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR);
}

bool createFolder(string folderPath) {
    if (mkdir(folderPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
        return false;
    }
    return true;
}


int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << "<ip_address> <port> <buffer_size>" << std::endl;
        return 1;
    }

    cout << ">>>>> Start!! <<<<<" << endl;
    std::string cur, next;
    bool flag_cur, flag_next;
    int idx = 0;

    std::string ping = "ping";
    std::string pong = "pong";
    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]); 
    int buffer_size = atoi(argv[3]);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cerr << "Failed to create socket" << endl;
        return 1;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    if (!isFolderExist(ping)) 
    {
        if (!createFolder(ping)) 
        {
            cout << "create 'ping' folder failed!!" << endl;
            return 1;
        }
        cout << "create 'ping' folder success!!" << endl;
    } 
    else 
    {
        for(int i=0;i < buffer_size;i++)
        {
            if(Is_File_Exist("ping/" + std::to_string(i) + "_hand.bin"))
            {
                remove(("ping/" + std::to_string(i) + "_hand.bin").c_str());
            }
        }

    }

    if (!isFolderExist(pong)) 
    {
        if (!createFolder(pong)) 
        {
            cout << "create 'pong' folder failed!!" << endl;
            return 1;
        }
        cout << "create 'pong' folder success!!" << endl;
    } 
    else 
    {
        for(int i=0;i < buffer_size;i++)
        {
            if(Is_File_Exist("pong/" + std::to_string(i) + "_hand.bin"))
            {
                remove(("pong/" + std::to_string(i) + "_hand.bin").c_str());
            }
        }

    }
    
    int isConnect = -1;
	while(isConnect < 0)
    {            
        isConnect = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));    
    }

    if(isConnect >= 0)
    {
        std::cout << "connect success" << std::endl;
    }

    std::string execute_folder = ping;
    std::string not_execute_folder = pong;
    while(1)
    {
		std::string execute_first_bin = execute_folder +  "/" + std::to_string(0) + "_hand.bin";
        std::string not_execute_first_bin = not_execute_folder +  "/" + std::to_string(0) + "_hand.bin";

        {
            
            if(idx >= buffer_size-2)
            {
                for(int i=idx; i<buffer_size; i++)
                {
                    if(Is_File_Exist(execute_folder +  "/" + std::to_string(i) + "_hand.bin"))
                    {
                        remove((execute_folder +  "/" + std::to_string(i) + "_hand.bin").c_str());
                    }
                }
                std::string tmp = execute_folder;
                execute_folder = not_execute_folder;
                not_execute_folder = tmp;
                idx = 0;
            }
        }


        {
            
            if(Is_File_Exist(not_execute_first_bin))
            {
                for(int i=idx; i<buffer_size; i++)
                {
                    if(Is_File_Exist(execute_folder +  "/" + std::to_string(i) + "_hand.bin"))
                    {
                        remove((execute_folder +  "/" + std::to_string(i) + "_hand.bin").c_str());
                    }
                }
                std::string tmp = execute_folder;
                execute_folder = not_execute_folder;
                not_execute_folder = tmp;
                idx = 0;
            }
        }
        cur = std::to_string(idx);
        flag_cur = Is_File_Exist(execute_folder +  "/" + cur + "_hand.bin");
        next = std::to_string(idx+1);
        flag_next = Is_File_Exist(execute_folder +  "/" + next + "_hand.bin");
        if(flag_cur && flag_next)
        {
            ScopedTiming st("trans time", 1);
            string out_bin_pth_file;
            vector<char>  dump_bin_(21*3*4);
            vector<float>  dump_bin_float;
            out_bin_pth_file = execute_folder +  "/" + cur + "_hand.bin";
            auto buff_float_0 = read_binary_file(out_bin_pth_file.c_str());

            dump_bin_float.insert(dump_bin_float.end(), buff_float_0.begin(), buff_float_0.end());

            memcpy(dump_bin_.data(), dump_bin_float.data(), sizeof(float) * 63);

            const char *data = dump_bin_.data();
            int data_size = 21*3*4;
            int block_size = 1024;
            int sent_bytes = 0;

            while (sent_bytes < data_size) {
                int bytes_to_send = min(block_size, data_size - sent_bytes);
                int bytes_sent = send(sock, data + sent_bytes, bytes_to_send, 0);
                if (bytes_sent < 0) {
                    cerr << "Failed to send data" << endl;
                    return 1;
                }
                sent_bytes += bytes_sent;
            }
            
            remove((execute_folder +  "/" + cur + "_hand.bin").c_str());
            idx++;
        }   
        else
        {
            continue;
        }
    }
    close(sock);

    return 0;
}
