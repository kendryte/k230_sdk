#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>


using namespace std;


int getBinSize(std::string path)
{
   int size = 0;
   std::ifstream infile(path, std::ifstream::binary);
   
   infile.seekg(0, infile.end);
   size= infile.tellg();
   infile.seekg(0, infile.beg);
   
   infile.close();
   cout<<"path= "<<path<<",size= "<<size<<endl;
   return size;

}





void readBin_float(std::string path, float* buf, int size)
{
 std::ifstream infile(path, std::ifstream::binary);

 infile.read(reinterpret_cast<char* >(buf), sizeof(float)*size);
 infile.close();
}

void readBin_int(std::string path, int* buf, int size)
{
 std::ifstream infile(path, std::ifstream::binary);

 infile.read(reinterpret_cast<char* >(buf), sizeof(int)*size);
 infile.close();
}

void writeBin_float(std::string path, float* buf,int size)
{
 std::ofstream infile(path, std::ifstream::binary);

 infile.write(reinterpret_cast<char* >(buf), sizeof(float)*size);
 infile.close();
}

void writeBin_int(std::string path, int* buf,int size)
{
 std::ofstream infile(path, std::ifstream::binary);

 infile.write(reinterpret_cast<char* >(buf), sizeof(int)*size);
 infile.close();
}


float getMold(float* vec ,int n){   //求向量的模长
        float sum = 0.0;
        for (int i = 0; i<n; ++i)
            sum += (vec[i] * vec[i]);
            
        return sqrt(sum);
    }

float getMold_int(int* vec ,int n){   //求向量的模长
        float sum = 0.0;
        for (int i = 0; i<n; ++i)
            sum += (vec[i] * vec[i]);
            
        return sqrt(sum);
    }

float getSimilarity(float* lhs, float* rhs,int n){
    float tmp = 0.0;  //内积
    for (int i = 0; i<n; ++i)
    {   
        // cout<<i<<" , lhs : "<<lhs[i]<<" , rhs : "<<rhs[i]<<endl;
        tmp += (lhs[i] * rhs[i]);

    }
    // cout<<"tmp : "<<tmp<<endl;
    // cout<<"getMold(lhs) : "<<getMold(lhs,n)<<" , getMold(rhs)"<<getMold(rhs,n)<<endl;
    
    return tmp / (getMold(lhs,n)*getMold(rhs,n));
}

float getSimilarity_int(int* lhs, int* rhs,int n){
    float tmp = 0.0;  //内积
    for (int i = 0; i<n; ++i)
    {   
        // cout<<i<<" , lhs : "<<lhs[i]<<" , rhs : "<<rhs[i]<<endl;
        tmp += (lhs[i] * rhs[i]);

    }
    // cout<<"tmp : "<<tmp<<endl;
    // cout<<"getMold(lhs) : "<<getMold(lhs,n)<<" , getMold(rhs)"<<getMold(rhs,n)<<endl;
    
    return tmp / (getMold_int(lhs,n)*getMold_int(rhs,n));
}


//估算系统内存
int GetSysMemInfo() {  //获取系统当前可用内存
       
        int mem_free = -1;//空闲的内存，=总内存-使用了的内存
        int mem_total = -1; //当前系统可用总内存
        int mem_buffers = -1;//缓存区的内存大小
        int mem_cached = -1;//缓存区的内存大小
        char name[20];
 
        FILE *fp;
        char buf1[128], buf2[128], buf3[128], buf4[128], buf5[128];
        int buff_len = 128;
        fp = fopen("/proc/meminfo", "r");
        if (fp == NULL) {
            std::cerr << "GetSysMemInfo() error! file not exist" << std::endl;
            return -1;
        }
        if (NULL == fgets(buf1, buff_len, fp) ||
            NULL == fgets(buf2, buff_len, fp) ||
            NULL == fgets(buf3, buff_len, fp) ||
            NULL == fgets(buf4, buff_len, fp) ||
            NULL == fgets(buf5, buff_len, fp)) {
            std::cerr << "GetSysMemInfo() error! fail to read!" << std::endl;
            fclose(fp);
            return -1;
        }
        fclose(fp);
        sscanf(buf1, "%s%d", name, &mem_total);
        sscanf(buf2, "%s%d", name, &mem_free);
        sscanf(buf4, "%s%d", name, &mem_buffers);
        sscanf(buf5, "%s%d", name, &mem_cached);
        int memLeft = mem_free + mem_buffers + mem_cached;
        return memLeft;
}

