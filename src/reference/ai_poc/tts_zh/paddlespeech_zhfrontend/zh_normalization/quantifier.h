#ifndef QUANTIFIER_H
#define QUANTIFIER_H

#include "num.h"


const std::regex RE_TEMPERATURE(R"((-?)(\d+(\.\d+)?)(°|°C|℃|度|摄氏度))");

std::string replace_temperature(std::smatch match);

#endif