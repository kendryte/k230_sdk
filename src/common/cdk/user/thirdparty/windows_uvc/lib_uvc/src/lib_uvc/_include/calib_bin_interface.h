#pragma once
//interface for K230
inline unsigned char decrypt_(unsigned char symbol1)
{
    symbol1 = (symbol1 << 3) | (symbol1 >> (8 - 3));
    symbol1 = symbol1 ^ (1 | (1 << 2) | (1 << 4) | (1 << 6));
    return symbol1;
}

struct temperature_info
{
    float temperature_ref;
    float temperature_cx;
    float temperature_cy;
    float kxppt;
    float kyppt;
    float reserve[11];
};

struct calib_data_file
{
    char param_bin_name[64];
    temperature_info info;//temperature_ref temperature_cx temperature_cy kxppt kyppt
    int ref_size_;//h*w*2 byte
    char* P_ref_;
    char* P_bin_params;
};
