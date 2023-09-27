#!/bin/bash
export MPP_SRC_DIR=$(pwd)
export RTSMART_SRC_DIR=${MPP_SRC_DIR}/../rt-smart
export RTT_CC=gcc
export RTT_CC_PREFIX=riscv64-unknown-linux-musl-
export RTT_EXEC_PATH=$(pwd)/../../../toolchain/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin
export PATH=$PATH:$RTT_EXEC_PATH

