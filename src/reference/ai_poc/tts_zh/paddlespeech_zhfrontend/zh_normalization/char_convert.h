#ifndef CHAR_CONVERT_H
#define CHAR_CONVERT_H
#include <iostream>
#include <string>  
#include <sstream>
#include <unordered_map>
#include <vector>
#include <locale>
#include <assert.h>
#include <wchar.h>
#include <codecvt>



using namespace std;

static unordered_map<wchar_t ,wchar_t >s2t_dict;
static unordered_map<wchar_t ,wchar_t >t2s_dict;

void Init();
wstring tranditional_to_simplified(wstring text);
// wstring simplified_to_traditional(wstring text);

#endif



