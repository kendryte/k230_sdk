#ifndef LENGTH_REGULATOR_H
#define LENGTH_REGULATOR_H

#include <stdio.h>
#include <iostream>
#include <iterator>
#include <vector>
#include <numeric>


using namespace std;

/*
*按照持续时间将encoder的输出重复扩展的可以被定长decoder使用的标准输出，包括扩展后的embedding和padding长度
*/
struct length_outputs
{
    vector<float> repeat_encoder_hidden_states;
    int M_pad;
} ;
length_outputs length_regulator(vector<float> encoder_buf,vector<float>,int N,int M_pad,int max_durations,int Dim);

#endif