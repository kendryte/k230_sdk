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

