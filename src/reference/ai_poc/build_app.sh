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

k230_bin=`pwd`/k230_bin
rm -rf ${k230_bin}
mkdir -p ${k230_bin}

cp -a ../../big/kmodel/ai_poc/kmodel/*.kmodel ${k230_bin}
cp -a ../../big/kmodel/ai_poc/images/* ${k230_bin}
cp -a shell/* ${k230_bin}

if [ -f out/bin/face_detection.elf ]; then
      cp out/bin/face_detection.elf ${k230_bin}
fi

if [ -f out/bin/face_landmark.elf ]; then
      cp out/bin/face_landmark.elf ${k230_bin}
fi

if [ -f out/bin/face_glasses.elf ]; then
      cp out/bin/face_glasses.elf ${k230_bin}
fi

if [ -f out/bin/face_mask.elf ]; then
      cp out/bin/face_mask.elf ${k230_bin}
fi

if [ -f out/bin/face_emotion.elf ]; then
      cp out/bin/face_emotion.elf ${k230_bin}
fi

if [ -f out/bin/face_pose.elf ]; then
      cp out/bin/face_pose.elf ${k230_bin}
fi

if [ -f out/bin/face_gender.elf ]; then
      cp out/bin/face_gender.elf ${k230_bin}
fi

if [ -f out/bin/sq_hand_det.elf ]; then
      cp out/bin/sq_hand_det.elf ${k230_bin}
fi

if [ -f out/bin/sq_handkp_class.elf ]; then
      cp out/bin/sq_handkp_class.elf ${k230_bin}
fi

if [ -f out/bin/sq_handkp_det.elf ]; then
      cp out/bin/sq_handkp_det.elf ${k230_bin}
fi

if [ -f out/bin/sq_handreco.elf ]; then
      cp out/bin/sq_handreco.elf ${k230_bin}
fi

if [ -f out/bin/ob_det.elf ]; then
      cp out/bin/ob_det.elf ${k230_bin}
fi

if [ -f out/bin/person_detect.elf ]; then
      cp out/bin/person_detect.elf ${k230_bin}
fi

if [ -f out/bin/falldown_detect.elf ]; then
      cp out/bin/falldown_detect.elf ${k230_bin}
fi

if [ -f out/bin/pose_detect.elf ]; then
      cp out/bin/pose_detect.elf ${k230_bin}
fi

rm -rf out
