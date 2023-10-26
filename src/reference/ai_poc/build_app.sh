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
mkdir ${k230_bin}/bytetrack_data/output
cp -a ../../big/kmodel/ai_poc/utils/* ${k230_bin}
cp -a shell/* ${k230_bin}

/opt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin/riscv64-unknown-linux-gnu-g++ -O3 llamac/llama_run.cc -o out/llama_run -lm

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

if [ -f out/bin/face_parse.elf ]; then
      cp out/bin/face_parse.elf ${k230_bin}
fi

if [ -f out/bin/head_detection.elf ]; then
      cp out/bin/head_detection.elf ${k230_bin}
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

if [ -f out/bin/sq_handkp_ocr.elf ]; then
      cp out/bin/sq_handkp_ocr.elf ${k230_bin}
fi

if [ -f out/bin/licence_det.elf ]; then
      cp out/bin/licence_det.elf ${k230_bin}
fi

if [ -f out/bin/licence_det_rec.elf ]; then
      cp out/bin/licence_det_rec.elf ${k230_bin}
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

if [ -f out/bin/person_attr.elf ]; then
      cp out/bin/person_attr.elf ${k230_bin}
fi

if [ -f out/bin/vehicle_attr.elf ]; then
      cp out/bin/vehicle_attr.elf ${k230_bin}
fi

if [ -f out/bin/pphumanseg.elf ]; then
      cp out/bin/pphumanseg.elf ${k230_bin}
fi

if [ -f out/bin/fitness.elf ]; then
      cp out/bin/fitness.elf ${k230_bin}
fi

if [ -f out/bin/bytetrack.elf ]; then
      cp out/bin/bytetrack.elf ${k230_bin}
fi

if [ -f out/bin/seg.elf ]; then
      cp out/bin/seg.elf ${k230_bin}
fi

if [ -f out/llama_run ]; then
      cp out/llama_run ${k230_bin}
fi

if [ -f out/bin/finger_guessing.elf ]; then
      cp out/bin/finger_guessing.elf ${k230_bin}
fi

if [ -f out/bin/smoke_detect.elf ]; then
      cp out/bin/smoke_detect.elf ${k230_bin}
fi

if [ -f out/bin/nanotracker.elf ]; then
      cp out/bin/nanotracker.elf ${k230_bin}
fi

if [ -f out/bin/traffic_light_detect.elf ]; then
      cp out/bin/traffic_light_detect.elf ${k230_bin}
fi

if [ -f out/bin/space_resize.elf ]; then
      cp out/bin/space_resize.elf ${k230_bin}
fi

if [ -f out/bin/eye_gaze.elf ]; then
      cp out/bin/eye_gaze.elf ${k230_bin}
fi

if [ -f out/bin/helmet_detect.elf ]; then
      cp out/bin/helmet_detect.elf ${k230_bin}
fi

if [ -f out/bin/puzzle_game.elf ]; then
      cp out/bin/puzzle_game.elf ${k230_bin}
fi

if [ -f out/bin/dynamic_gesture.elf ]; then
      cp out/bin/dynamic_gesture.elf ${k230_bin}
fi

if [ -f out/bin/ocr_reco.elf ]; then
      cp out/bin/ocr_reco.elf ${k230_bin}
fi

if [ -f out/bin/yolop.elf ]; then
      cp out/bin/yolop.elf ${k230_bin}
fi

if [ -f out/bin/anomaly_det.elf ]; then
      cp out/bin/anomaly_det.elf ${k230_bin}
fi

rm -rf out
