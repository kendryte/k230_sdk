#!/bin/bash
### 7.4.0
#export PATH=$PATH:/data/zhanglimin/code_kmodel_export_build_inference/AndeSight_STD_v323/toolchains/nds64le-linux-glibc-v5d/bin
### 7.3.0
export PATH=$PATH:../../../../toolchain/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin/
rm -rf build
mkdir build
pushd build
cmake -DCMAKE_BUILD_TYPE=Release                  \
      -DCMAKE_INSTALL_PREFIX=`pwd`                \
      -DCMAKE_TOOLCHAIN_FILE=../../../../src/big/nncase/examples/cmake/Riscv64.cmake \
      ..
      
#make -j && make install
make -j 



popd

