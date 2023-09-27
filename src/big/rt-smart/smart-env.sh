#!/bin/bash

# usage:
# source smart-env.sh [arch]
# example: source smart-env.sh          # arm
# example: source smart-env.sh aarch64  # aarch64

# supported arch list
supported_arch="arm aarch64 riscv64 i386"

def_arch="unknown" 

# find arch in arch list
if [ -z $1 ]
then
    def_arch="arm" # default arch is arm
else
    for arch in $supported_arch
    do
        if [ $arch = $1 ]
        then
            def_arch=$arch
            break
        fi
    done
fi

# set env
case $def_arch in 
    "arm")
        export RTT_CC=gcc
        export RTT_EXEC_PATH=~/.tools/gnu_gcc/arm-linux-musleabi_for_x86_64-pc-linux-gnu/bin
        export RTT_CC_PREFIX=arm-linux-musleabi-
        cp userapps/configs/def_config_arm userapps/.config
        ;;
    "aarch64")
        export RTT_CC=gcc
        export RTT_EXEC_PATH=~/.tools/gnu_gcc/aarch64-linux-musleabi_for_x86_64-pc-linux-gnu/bin
        export RTT_CC_PREFIX=aarch64-linux-musleabi-
        cp userapps/configs/def_config_aarch64 userapps/.config
        ;;
    "riscv64")
        export RTT_CC=gcc
        export RTT_EXEC_PATH=$(pwd)/../../../toolchain/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin
        export RTT_CC_PREFIX=riscv64-unknown-linux-musl-
        export RTT_SDK_BUILD_DIR=build
        export MPP_SRC_DIR=$(pwd)/../mpp
        export CDK_SRC_DIR=$(pwd)/../../common/cdk
        cp userapps/configs/def_config_riscv64 userapps/.config
        ;;
    "i386")
        export RTT_CC=gcc
        export RTT_EXEC_PATH=~/.tools/gnu_gcc/i386-linux-musleabi_for_x86_64-pc-linux-gnu/bin
        export RTT_CC_PREFIX=i386-unknown-linux-musl-
        ;;
    *)  echo "unknown arch!"
        return 1
esac

# export RTT_EXEC_PATH
export PATH=~/.env/tools/scripts:$PATH:$RTT_EXEC_PATH

echo "Arch         => ${def_arch}"
echo "CC           => ${RTT_CC}"
echo "PREFIX       => ${RTT_CC_PREFIX}"
echo "EXEC_PATH    => ${RTT_EXEC_PATH}"
