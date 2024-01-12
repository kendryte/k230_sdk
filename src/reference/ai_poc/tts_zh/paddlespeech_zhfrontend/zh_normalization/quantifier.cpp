#include "quantifier.h"


std::string replace_temperature(smatch match) {
    std::string sign = match[1].str();
    std::string temperature = match[2].str();
    std::string unit = match[3].str();
    string result;
    if (sign == "-") {
        sign = "零下";
    }
    temperature = num2str(temperature);
    if ((unit == "摄氏度")|(unit == "℃")|(unit == "°C")){
        unit = "摄氏度";
    }
    else
    {
        unit = "度";
    }
    
    result = sign + temperature + unit;
    return result;
}


// int main()
// {
    
//     string text = "-30° 20° 15℃ -20/30和55/66，5%。-6%。00034,345 00010000 20~0.5 20+朵花 30多岁 2:15:30  4:20:10-5:16:10 2020年3月4日 2010/03/04  2250-01-03";
    
//     text = replace(replace_temperature,text,RE_TEMPERATURE);
   
    
//     cout<<text<<endl;
   
//     return 0;
// }
