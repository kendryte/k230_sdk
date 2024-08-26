#!/bin/bash
# set -x

k230_bin=`pwd`/k230_bin

# 定义.h文件的路径
header_file="../../../src/big/mpp/include/comm/k_autoconf_comm.h"

is_01studio=0
if grep -q "k230_canmv_01studio" "$header_file"; then
    echo "宏定义 k230_canmv_01studio 存在。"
    is_01studio=1
    display_mode="hdmi"
    if [ $# -eq 2 ]; then
      if [[ "$2" == "lcd" ]]; then
        display_mode=$2
        k230_bin=$(pwd)/k230_bin_${display_mode}
      fi
    fi
else
    echo "不是k230_canmv_01studio"
fi

echo ${k230_bin}
rm -rf ${k230_bin}
mkdir -p ${k230_bin}

# set cross build toolchain
export PATH=$PATH:/opt/toolchain/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin/

clear
rm -rf out
mkdir out

subdirs=$(find . -mindepth 1 -maxdepth 1 -type d)

if [ -z "$1" ]; then
    curr_pro="all"
else
    curr_pro="$1"
fi

echo "$curr_pro"

for subdir in $(ls -d */); do
      if [ ! -d "$subdir" ]; then
        echo "$subdir 不是一个目录，跳过..."
        continue
      fi
      
      subdir_name=$(basename $subdir)
      # 检查子目录是否为目录"A"，如果是，则跳过
      if [ "$subdir_name" = "cmake" ] || [[ "$subdir_name" == k230_bin* ]] || [ "$subdir_name" = "shell" ] || [ "$subdir_name" = "out" ]; then
            continue
      fi
      
      if [ "$subdir_name" = "llamac" ] && { [ "$curr_pro" = "llamac" ] || [ "$curr_pro" = "all" ]; }; then
            /opt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin/riscv64-unknown-linux-gnu-g++ -O3 llamac/llama_run.cc -o out/llama_run -lm
      elif { [ "$curr_pro" = "$subdir_name" ] || [ "$curr_pro" = "all" ]; }; then
            echo "******************$subdir_name 开始编译******************"
            if [ "$subdir_name" = "pose_det_rtsp_plug" ]; then
                  cd pose_det_rtsp_plug/little/
                  ./build.sh
                  cd ../../
            elif [ "$subdir_name" = "translate_en_ch" ]; then
                  cp -a ../../big/kmodel/ai_poc/utils/*.a ${k230_bin}
            fi
            pushd out
            if [ "$is_01studio" -eq 1 ]; then
                  cmake -DCMAKE_BUILD_TYPE=Release                \
                  -DCMAKE_INSTALL_PREFIX=`pwd`               \
                  -DCMAKE_TOOLCHAIN_FILE=cmake/Riscv64.cmake \
                  -D$subdir_name=ON                        \
                  -DDISPLAY_MODE=${display_mode}                 \
                  ..
            else
                  cmake -DCMAKE_BUILD_TYPE=Release                \
                  -DCMAKE_INSTALL_PREFIX=`pwd`               \
                  -DCMAKE_TOOLCHAIN_FILE=cmake/Riscv64.cmake \
                  -D$subdir_name=ON                        \
                  ..
            fi
            make -j && make install
            echo "******************$subdir_name 编译完成******************"
            popd
      else
          continue  
      fi

      mkdir -p ${k230_bin}/$subdir_name
      if [ "$subdir_name" = "face_detection" ]; then
            cp out/bin/face_detection.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_640.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1024x624.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_landmark" ]; then
            cp out/bin/face_landmark.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_landmark.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1024x1331.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_landmark_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_glasses" ]; then
            cp out/bin/face_glasses.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_glasses.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_glasses_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_mask" ]; then
            cp out/bin/face_mask.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_mask.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_mask_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_emotion" ]; then
            cp out/bin/face_emotion.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_emotion.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_emotion_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_pose" ]; then
            cp out/bin/face_pose.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_pose.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_pose_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_gender" ]; then
            cp out/bin/face_gender.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_gender.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_gender_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_parse" ]; then
            cp out/bin/face_parse.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_parse.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_parse_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "head_detection" ]; then
            cp out/bin/head_detection.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/head_detection.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/640x340.jpg ${k230_bin}/$subdir_name
            cp -a shell/head_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "sq_hand_det" ]; then
            cp out/bin/sq_hand_det.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/input_hd.jpg ${k230_bin}/$subdir_name
            cp -a shell/handdet_cpp_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "sq_handkp_class" ]; then
            cp out/bin/sq_handkp_class.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/input_hd.jpg ${k230_bin}/$subdir_name
            cp -a shell/handkpclass_cpp_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "sq_handkp_det" ]; then
            cp out/bin/sq_handkp_det.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/input_hd.jpg ${k230_bin}/$subdir_name
            cp -a shell/handkpdet_cpp_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "sq_handreco" ]; then
            cp out/bin/sq_handreco.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_reco.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/input_hd.jpg ${k230_bin}/$subdir_name
            cp -a shell/handreco_cpp_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "sq_handkp_ocr" ]; then
            cp out/bin/sq_handkp_ocr.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/ocr_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/ocr_rec.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/input_ocr.jpg ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/HZKf2424.hz ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/Asci0816.zf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/dict_6625.txt ${k230_bin}/$subdir_name
            cp -a shell/handkpocr_cpp_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "licence_det" ]; then
            cp out/bin/licence_det.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/LPD_640.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/licence.jpg ${k230_bin}/$subdir_name
            cp -a shell/licence_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "licence_det_rec" ]; then
            cp out/bin/licence_det_rec.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/LPD_640.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/licence_reco.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/licence.jpg ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/HZKf2424.hz ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/Asci0816.zf ${k230_bin}/$subdir_name
            cp -a shell/licence_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "object_detect_yolov8n" ]; then
            cp out/bin/ob_det.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/yolov8n_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/yolov8n_640.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/bus.jpg ${k230_bin}/$subdir_name            
            cp -a shell/ob_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "person_detect" ]; then
            cp out/bin/person_detect.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/person_detect_yolov5n.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/bus.jpg ${k230_bin}/$subdir_name            
            cp -a shell/person_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "falldown_detect" ]; then
            cp out/bin/falldown_detect.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/yolov5n-falldown.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/falldown_elder.jpg ${k230_bin}/$subdir_name            
            cp -a shell/falldown_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "pose_detect" ]; then
            cp out/bin/pose_detect.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/yolov8n-pose.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/bus.jpg ${k230_bin}/$subdir_name            
            cp -a shell/pose_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "person_attr" ]; then
            cp out/bin/person_attr.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/person_attr_yolov5n.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/person_pulc.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/hrnet_demo.jpg ${k230_bin}/$subdir_name            
            cp -a shell/person_attr_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "vehicle_attr" ]; then
            cp out/bin/vehicle_attr.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/vehicle_attr_yolov5n.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/vehicle.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/car.jpg ${k230_bin}/$subdir_name            
            cp -a shell/vehicle_attr_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "pphumanseg" ]; then
            cp out/bin/pphumanseg.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/human_seg_2023mar.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1000.jpg ${k230_bin}/$subdir_name            
            cp -a shell/pphumanseg_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "fitness" ]; then
            cp out/bin/fitness.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/yolov8n-pose.kmodel ${k230_bin}/$subdir_name
            cp -a shell/fitness_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "bytetrack" ]; then
            cp out/bin/bytetrack.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/bytetrack_yolov5n.kmodel ${k230_bin}/$subdir_name
            cp -ar ../../big/kmodel/ai_poc/images/bytetrack_data ${k230_bin}/$subdir_name
            cp -a shell/bytetrack_*.sh ${k230_bin}/$subdir_name
            mkdir -p ${k230_bin}/$subdir_name/bytetrack_data/output
      fi

      if [ "$subdir_name" = "segment_yolov8n" ]; then
            cp out/bin/seg.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/yolov8n_seg_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/yolov8n_seg_640.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/bus.jpg ${k230_bin}/$subdir_name 
            cp -a shell/segment_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "llamac" ]; then
            cp out/llama_run ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/llama.bin ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/tokenizer.bin ${k230_bin}/$subdir_name
            cp -a shell/llama_build.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "finger_guessing" ]; then
            cp out/bin/finger_guessing.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/shitou.bin ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/bu.bin ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/jiandao.bin ${k230_bin}/$subdir_name
            cp -a shell/finger_guessing_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "smoke_detect" ]; then
            cp out/bin/smoke_detect.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/yolov5s_smoke_best.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/smoke1.jpg ${k230_bin}/$subdir_name 
            cp -a shell/smoke_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "nanotracker" ]; then
            cp out/bin/nanotracker.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/cropped_test127.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/nanotrack_backbone_sim.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/nanotracker_head_calib_k230.kmodel ${k230_bin}/$subdir_name
            cp -a shell/nanotracker_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "traffic_light_detect" ]; then
            cp out/bin/traffic_light_detect.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/traffic.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/traffic.jpg ${k230_bin}/$subdir_name 
            cp -a shell/traffic_light_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "space_resize" ]; then
            cp out/bin/space_resize.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a shell/space_resize_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "eye_gaze" ]; then
            cp out/bin/eye_gaze.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/eye_gaze.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1024x1111.jpg ${k230_bin}/$subdir_name 
            cp -a shell/eye_gaze_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "helmet_detect" ]; then
            cp out/bin/helmet_detect.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/helmet.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/helmet.jpg ${k230_bin}/$subdir_name 
            cp -a shell/helmet_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "puzzle_game" ]; then
            cp out/bin/puzzle_game.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/pintu.bin ${k230_bin}/$subdir_name
            cp -a shell/puzzle_game_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "dynamic_gesture" ]; then
            cp out/bin/dynamic_gesture.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/gesture.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/shang.bin ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/xia.bin ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/zuo.bin ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/you.bin ${k230_bin}/$subdir_name
            cp -a shell/gesture*.sh ${k230_bin}/$subdir_name
      fi
      
      if [ "$subdir_name" = "ocr" ]; then
            cp out/bin/ocr_reco.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/ocr_det_int16.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/ocr_rec_int16.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/333.jpg ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/dict_ocr.txt  ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/dict_ocr_16.txt  ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/HZKf2424.hz ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/Asci0816.zf ${k230_bin}/$subdir_name
            cp -a shell/ocr_*.sh ${k230_bin}/$subdir_name
      fi
      
      if [ "$subdir_name" = "yolop" ]; then
            cp out/bin/yolop.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/yolop.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/333.jpg ${k230_bin}/$subdir_name 
            cp -a shell/yolop_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "anomaly_det" ]; then
            cp out/bin/anomaly_det.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/anomaly_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/000.png ${k230_bin}/$subdir_name 
            cp -a ../../big/kmodel/ai_poc/utils/memory.bin ${k230_bin}/$subdir_name
            cp -a shell/anomaly_det_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_mesh" ]; then
            cp out/bin/face_mesh.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_alignment.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_alignment_post.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1024x768.jpg ${k230_bin}/$subdir_name 
            cp -a ../../big/kmodel/ai_poc/utils/bfm_tri.bin ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/ncc_code.bin ${k230_bin}/$subdir_name
            cp -a shell/face_mesh_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_alignment" ]; then
            cp out/bin/face_alignment.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_alignment.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_alignment_post.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/1024x768.jpg ${k230_bin}/$subdir_name 
            cp -a ../../big/kmodel/ai_poc/utils/bfm_tri.bin ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/ncc_code.bin ${k230_bin}/$subdir_name
            cp -a shell/face_alignment_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "translate_en_ch" ]; then
            cp out/bin/translate_en_ch.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/translate_encoder.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/translate_decoder.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/trans_src.model ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/trans_tag.model ${k230_bin}/$subdir_name
            cp -a shell/translate_en_ch.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "self_learning" ]; then
            cp out/bin/self_learning.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/recognition.kmodel ${k230_bin}/$subdir_name
            cp -a shell/self_learning.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "crosswalk_detect" ]; then
            cp out/bin/crosswalk_detect.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/crosswalk.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/cw.jpg ${k230_bin}/$subdir_name 
            cp -a shell/crosswalk_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "virtual_keyboard" ]; then
            cp out/bin/virtual_keyboard.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a shell/virtual_keyboard.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "sq_handkp_flower" ]; then
            cp out/bin/sq_handkp_flower.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/flower_rec.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/input_flower.jpg ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/HZKf2424.hz ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/Asci0816.zf ${k230_bin}/$subdir_name 
            cp -a shell/handkpflower_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_verification" ]; then
            cp out/bin/face_verification.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_recognition.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/identification_card.png ${k230_bin}/$subdir_name 
            cp -a ../../big/kmodel/ai_poc/images/person.png ${k230_bin}/$subdir_name 
            cp -a shell/face_verification_*.sh ${k230_bin}/$subdir_name
      fi
      
      if [ "$subdir_name" = "dms_system" ]; then
            cp out/bin/dms.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a shell/dms_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "distraction_reminder" ]; then
            cp out/bin/distraction_reminder.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_pose.kmodel ${k230_bin}/$subdir_name
            cp -a shell/distraction_reminder_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "kws" ]; then
            cp out/bin/kws.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/kws.kmodel ${k230_bin}/$subdir_name
            cp -ar ../../big/kmodel/ai_poc/utils/reply_wav/ ${k230_bin}/$subdir_name
            cp -a shell/kws.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "tts_zh" ]; then
            cp out/bin/tts_zh.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/zh_fastspeech_1.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/zh_fastspeech_2.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hifigan.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/utils/wav_play.elf ${k230_bin}/$subdir_name 
            cp -ar ../../big/kmodel/ai_poc/utils/file ${k230_bin}/$subdir_name 
            cp -a shell/tts_zh.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "person_distance" ]; then
            cp out/bin/person_distance.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/person_detect_yolov5n.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/input_pd.jpg ${k230_bin}/$subdir_name
            cp -a shell/person_distance_*.sh ${k230_bin}/$subdir_name
      fi
      
      if [ "$subdir_name" = "demo_mix" ]; then
            cp out/bin/demo_mix.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/gesture.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_pose.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/nanotrack_backbone_sim.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/nanotracker_head_calib_k230.kmodel ${k230_bin}/$subdir_name
            cp -a shell/demo_mix.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "dec_ai_enc" ]; then
            cp out/bin/dec_enc.elf ${k230_bin}/$subdir_name
            cp out/bin/dec.elf ${k230_bin}/$subdir_name
            cp out/bin/enc.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/face_detection_hwc.kmodel ${k230_bin}/$subdir_name
            cp -a shell/dec*.sh ${k230_bin}/$subdir_name
            cp -a shell/enc*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "yolop_lane_seg" ]; then
            cp out/bin/yolop.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/yolop.kmodel ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/images/road.jpg ${k230_bin}/$subdir_name
            cp -a shell/yolop_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "pose_det_rtsp_plug" ]; then
            cp pose_det_rtsp_plug/little/out/rtspServer ${k230_bin}/$subdir_name 
            cp out/bin/pose_det_enc.elf ${k230_bin}/$subdir_name
            cp -a ../../big/kmodel/ai_poc/kmodel/yolov8n-pose.kmodel ${k230_bin}/$subdir_name
            cp -a shell/rtsp_plug_*.sh ${k230_bin}/$subdir_name
      fi
      rm -rf out/*
done

rm -rf out
