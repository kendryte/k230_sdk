#!/bin/bash
set -x

# set cross build toolchain
export PATH=$PATH:/opt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin
    
# @export PATH=$(RTT_EXEC_PATH):$(PATH);export RTSMART_SRC_DIR=$(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH);\
# cd $(K230_SDK_ROOT)/src/reference/meta_human/datafifo_big; \


clear
rm -rf out
mkdir out
pushd out
cmake -DCMAKE_BUILD_TYPE=Release                 \
      -DCMAKE_INSTALL_PREFIX=`pwd`               \
      -DCMAKE_TOOLCHAIN_FILE=cmake/Riscv64.cmake \
      ..

make -j && make install
popd