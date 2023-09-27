#!/bin/bash
export CDK_SRC_DIR=$(pwd)/..
export MPP_SRC_DIR=$(pwd)/../../../big/mpp
export RTT_CC=gcc
export RTT_CC_PREFIX=riscv64-unknown-linux-musl-
export RTT_EXEC_PATH=$(pwd)/../../../../toolchain/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin
export LINUX_EXEC_PATH=$(pwd)/../../../../toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin
export PATH=$PATH:$RTT_EXEC_PATH:$LINUX_EXEC_PATH


