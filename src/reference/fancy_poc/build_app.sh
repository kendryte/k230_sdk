#!/bin/bash
set -x

# set cross build toolchain
export PATH=$PATH:/opt/toolchain/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin/

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

# assemble all test cases
k230_bin=`pwd`/k230_bin

mkdir -p ${k230_bin}
if [ -f out/bin/AIOT_GPT.elf ]; then
      cp out/bin/AIOT_GPT.elf ${k230_bin}
fi
if [ -f out/bin/meta_human.elf ]; then
      cp out/bin/meta_human.elf ${k230_bin}
fi
if [ -f out/bin/ocr_poc.elf ]; then
      cp out/bin/ocr_poc.elf ${k230_bin}
fi
if [ -f out/bin/housekeeper.elf ]; then
      cp out/bin/housekeeper.elf ${k230_bin}
fi
if [ -f out/bin/meta_hand.elf ]; then
      cp out/bin/meta_hand.elf ${k230_bin}
fi
if [ -f out/bin/face_registration.elf ]; then
      cp out/bin/face_registration.elf ${k230_bin}
fi

if [ -f out/bin/ai_scale.elf ]; then
      cp out/bin/ai_scale.elf ${k230_bin}
fi

if [ -f out/bin/face_recognition.elf ]; then
      cp out/bin/face_recognition.elf ${k230_bin}
fi

/opt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin/riscv64-unknown-linux-gnu-g++ ./multimodal_chat_robot/src/multimodal_chat_robot_client/socket_gpt/main.cc -o ${k230_bin}/socket_gpt_image_audio
/opt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin/riscv64-unknown-linux-gnu-g++ ./meta_human/src/http/main.cc -o ${k230_bin}/http
/opt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin/riscv64-unknown-linux-gnu-g++ ./meta_hand/src/http/main.cc -o ${k230_bin}/connect
/opt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin/riscv64-unknown-linux-gnu-g++ ./finger_reader/src/finger_reader_client/finger_reader_comm/main.cc -o ${k230_bin}/Finger_Read_Poc
/opt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin/riscv64-unknown-linux-gnu-g++ ./housekeeper/src/housekeeper_client/housekeeper_comm/main.cc -o ${k230_bin}/Housekeeper
/opt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin/riscv64-unknown-linux-gnu-g++ ./ai_scale/src/ai_scale_client/client.cc -o ${k230_bin}/client
rm -rf out