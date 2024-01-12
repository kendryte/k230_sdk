#include <stdio.h>
#include <iostream>
#include <iterator>
#include <vector>
#include <numeric>
// #include  "utils.h"

using namespace std;

/*
*按照时间长度重复扩展的输出，包括扩展后的embedding和padding长度
*/
struct length_outputs
{
    vector<float> repeat_encoder_hidden_states;
    int M_pad;
};

/**
*encoder_buf: encoder编码得到的向量
*durations_buf: encoder输出，每个音素持续时间数据
*N：有效音素序列长度，<=50
*M_pad：补0个数初始值，0
*max_duritions：最大持续时长，即encoder输出按照音素持续时间重复扩展后的最大长度，600
*Dim：encoder将每个音素序列中的值编码成向量的维度，256
****note：因不支持变长输入，持续时间扩展这一步需要单独拿出来完成，并通过padding将其处理成定长的
*/
length_outputs length_regulator(vector<float> encoder_buf,vector<float> durations_buf,int N,int M_pad,int max_durations,int Dim)
{
    vector<int> durations ;
     for (const float& value : durations_buf) {
        int int_value = static_cast<int>(value); 
        durations.push_back(int_value);
    }
    //durations求和
    int real_length = accumulate(durations.begin() , durations.end() , 0);
    //对于real_length超过定长的处理
    //每个音素的最大持续时间不得超过13，依次将超过max_value的截断处理，max_value减一，计算总长度，如果仍然超过600，继续做截断处理，直到符合要求
    int max_value = 13;
    while(real_length>max_durations)
    {
        //对于大于上边界的直接取上边界
		for(int i=0;i<durations.size();i++)
		{
			if(durations[i]>max_value){
                durations[i]=max_value;
            }
		}
    	max_value = max_value-1;
    	real_length = accumulate(durations.begin() , durations.end() , 0);
    }
    //补0个数，计算和600的差距
    M_pad = max_durations - real_length;
    //定义padding之后decoder部分的真正输出，将数据拷贝到里面
    vector<float> repeat_encoder_hidden_states;
    for(int i=0;i<N;i++)
    {
        if(durations[i]>0)
        {
            for(int m=0;m<durations[i];m++)
            {
                //循环复制元素
                for(int j=0;j<Dim;j++)
                {   
                    repeat_encoder_hidden_states.push_back(encoder_buf[i*Dim+j]);
                }
            }
        }
    }
    //不足600的补0
    if(M_pad>0)
    {
        vector<float> data(Dim, 0);
        for(int i=0;i<M_pad;i++)
        {   
            for(int j=0;j<Dim;j++)
            {
                repeat_encoder_hidden_states.push_back(data[j]);
            }
        }
    }
    length_outputs length_output;
    length_output.repeat_encoder_hidden_states = repeat_encoder_hidden_states;
    length_output.M_pad = M_pad;
    return length_output;
    
}