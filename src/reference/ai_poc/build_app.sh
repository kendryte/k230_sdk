#!/bin/bash
# set -x

# 定义.h文件的路径
header_file="../../../src/big/mpp/include/comm/k_autoconf_comm.h"

if grep -q "k230_canmv_01studio" "$header_file"; then
    if [ $# -eq 0 ]; then
        ./build_app_sub.sh all hdmi
        ./build_app_sub.sh all lcd
    else
        if [ $# -eq 1 ]; then
            ./build_app_sub.sh "$1"
        elif [ $# -eq 2 ]; then
            ./build_app_sub.sh "$1" "$2"
        else
            echo "参数太多，最多支持两个参数"
        fi
    fi
else
    if [ $# -eq 0 ]; then
        ./build_app_sub.sh
    elif [ $# -eq 1 ]; then
        ./build_app_sub.sh "$1"
    else
         echo "参数太多，最多支持一个参数"
    fi
fi