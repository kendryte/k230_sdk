#include <iostream>
#include <vector>
#include <fstream>
#include <sys/stat.h>

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
	//ifs->vec
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

int main(int argc, char *argv[]) 
{

    std::string vec_dir = "vectors";
    if (!isFolderExist(vec_dir)) 
    {
        if (!createFolder(vec_dir)) 
        {
            std::cout << "Create 'vectors' folder failed!!" << std::endl;
            return 1;
        }
        std::cout << "Create 'vectors' folder success!!" << std::endl;
    } 
    else
    {
        std::cout <<  " 'vectors' folder has existed already!!" << std::endl;
    }
    

    while(1)
    {
        std::cout << "Please input the category:" << std::endl;
        std::string category = "";
        getline(std::cin, category);

        std::string cate_dir = vec_dir + "/" + category ; 
        if (!isFolderExist(cate_dir)) 
        {
            if (!createFolder(cate_dir)) 
            {
                std::cout << "Create " << "'" << cate_dir << "'" << " folder failed!!" << std::endl;
                return 1;
            }
            std::cout <<  "Create " << "'" << cate_dir << "'"  << " folder success!!" << std::endl;
            if(!Is_File_Exist( cate_dir +  "/index.txt" ))
            {
                fstream f;
                f.open((cate_dir +  "/index.txt" ),ios::out);
                f<<"0";
                f.close();
            }
        } 
        else
        {
            std::cout <<  "'" << cate_dir  << "'" << " folder has existed already!!" << std::endl;
        }
        
    }


    return 0;
}
