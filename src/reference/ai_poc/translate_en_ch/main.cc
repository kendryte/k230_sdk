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
#include <chrono>
#include <fstream>
#include <thread>

#include "etranslate.h"
#include "dtranslate.h"


void print_usage(const char *name)
{
	cout << "Usage: " << name << "<kmodel_encoder> <kmodel_decoder> <src_model_file> <tag_model_file> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_encoder      编码器kmodel路径\n"
         << "  kmodel_decoder      解码器kmodel路径\n"
         << "  src_model_file      原语言分词器模型路径\n"
		 << "  tag_model_file      翻译后语言分词器模型路径\n"
		 << "  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
		 << "\n"
		 << endl;
}

void translate_proc(char *argv[])
{
    ETranslate encoder(argv[1], argv[3], atoi(argv[5]));
    sync();
    DTranslate decoder(argv[2], argv[4], atoi(argv[5]));
    sync();

    char src_str[400];

    std::vector<int> int_res;
    vector<EncoderStruct> eresult;

    while (true)
    {
        std::cout << "Please enter : " << "(Enter q End)" << std::endl;
        gets(src_str);

        ScopedTiming st("total time", atoi(argv[5]));
        if(string(src_str) == "q")
        {
            break;
        }

        int_res.clear();
        eresult.clear();
    
        encoder.pre_process(string(src_str));
        encoder.inference();
        encoder.post_process(eresult);

        int maxlen = encoder.get_maxlen();
        vector<int64_t> input_y;
        bool tran_stop = false;
        for(int i = 0; i < maxlen-1; i++)
        {
            decoder.pre_process(i, input_y, eresult[0].encoder_kv, eresult[0].src_token_size);
            decoder.inference();
            decoder.post_process(i+1, input_y,int_res, &tran_stop);
            if (tran_stop)
            {
                break;
            }
        }

        string translate_str = decoder.decode_post(int_res);
        std::cout << "Result : " << translate_str << std::endl;
    }

}


int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc != 6)
    {
        print_usage(argv[0]);
        return -1;
    }

    translate_proc(argv);
    return 0;
}
