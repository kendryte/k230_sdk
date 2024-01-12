#include "jieba_utils.h"

const char* const DICT_PATH = "file/cppjieba_dict/jieba.dict.utf8";
const char* const HMM_PATH = "file/cppjieba_dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "file/cppjieba_dict/user.dict.utf8";
const char* const IDF_PATH = "file/cppjieba_dict/idf.utf8";
const char* const STOP_WORD_PATH = "file/cppjieba_dict/stop_words.utf8";
// const char* const dict_path = "file/pinyin_txt/pinyin.txt";
// const char* const phase_path = "file/pinyin_txt/large_pinyin.txt";


// cppjieba::Jieba jieba(DICT_PATH,
//     HMM_PATH,
//     USER_DICT_PATH,
//     IDF_PATH,
//     STOP_WORD_PATH);

Pypinyin pypinyin;

