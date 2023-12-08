cmake_minimum_required(VERSION 3.2)
set(SRC main.cpp vo/vo.cc audio/audio_buf_play.cc)

set(CMAKE_CXX_STANDARD 17)
set(bin dictionary_pen.elf)

set(nncase_sdk_root "${PROJECT_SOURCE_DIR}/../../../big/nncase")
set(k230_sdk ${nncase_sdk_root}/../../../)

set(CMAKE_EXE_LINKER_FLAGS "-T ${nncase_sdk_root}/examples/cmake/link.lds --static")


#riscv64交叉编译
include_directories(${k230_sdk}/src/big/utils/lib/opencv/include/opencv4)
include_directories(${k230_sdk}/src/big/utils/lib/freetype/include/freetype2)
#include_directories(${k230_sdk}/src/big/nncase/riscv64/sentencepiece/include/)
include_directories(${k230_sdk}/src/big/nncase/riscv64/)
include_directories(${k230_sdk}/src/big/nncase/riscv64/nncase/include/)

#sentencepiece
include_directories(${k230_sdk}/src/big/utils/lib/sentencepiece/include/)


link_directories(${k230_sdk}/src/big/utils/lib/opencv/lib)
link_directories(${k230_sdk}/src/big/utils/lib/opencv/lib/opencv4/3rdparty)
link_directories(${k230_sdk}/src/big/utils/lib/freetype/lib)
#link_directories(${k230_sdk}/src/big/nncase/riscv64/sentencepiece/lib/)
include_directories(${k230_sdk}/src/big/nncase/riscv64/nncase/include/nncase/runtime)
link_directories(${k230_sdk}/src/big/nncase/riscv64/nncase/lib/)

#sentencepiece
link_directories(${k230_sdk}/src/big/utils/lib/sentencepiece/lib)


#add vo audio mapi and include 
include_directories(${k230_sdk}/src/big/mpp/userapps/api/)
include_directories(${k230_sdk}/src/big/mpp/include)
include_directories(${k230_sdk}/src/big/mpp/include/comm)
include_directories(./)
include_directories(./audio)
include_directories(./vo)
include_directories(./vicap)

## set mmz
link_directories(${k230_sdk}/src/big/mpp/userapps/lib)

#添加openblas库
link_directories(${k230_sdk}/src/big/utils/lib/openblas/lib/)
include_directories(${k230_sdk}/src/big/utils/lib/openblas/include/)
    

 #imagestitch
include_directories(./include/stitch/include)

add_library(stitch STATIC ./include/stitch/src/stitch.cpp)
target_link_libraries(stitch opencv_imgcodecs  opencv_imgproc opencv_core  libjpeg-turbo libopenjp2 libpng libtiff libwebp zlib csi_cv)

# ocr
include_directories(./include/det_ocr/include)
#link_directories(./det_ocr/lib/)
add_library(detocr STATIC ./include/det_ocr/src/clipper.cpp ./include/det_ocr/src/det_util.cpp ./include/det_ocr/src/det_ocr.cpp)
target_link_libraries(detocr nncase.rt_modules.k230 Nncase.Runtime.Native opencv_imgcodecs  opencv_imgproc opencv_core  libjpeg-turbo libopenjp2 libpng libtiff libwebp zlib csi_cv)


include_directories(./include/)
#tts
include_directories(./include/tts/src)
include_directories(./include/tts/paddlespeech_zhfrontend)
include_directories(./include/tts/paddlespeech_zhfrontend/pypinyin)
include_directories(./include/tts/paddlespeech_zhfrontend/zh_normalization)
INCLUDE_DIRECTORIES(./include/tts/paddlespeech_zhfrontend/cppjieba/include
./include/tts/paddlespeech_zhfrontend/cppjieba/deps/limonp/include)
INCLUDE_DIRECTORIES(./include/tts/g2p_en)
INCLUDE_DIRECTORIES(./include/tts/g2p_en/nltk)
#
#
ADD_SUBDIRECTORY(./include/tts/paddlespeech_zhfrontend)
ADD_SUBDIRECTORY(./include/tts/g2p_en)

include_directories(./include/API)
aux_source_directory(./include/tts/src DIR_SRCS)
aux_source_directory(./include/API DIR_APIS)
add_executable(${bin} ${SRC} ${DIR_SRCS} ${DIR_APIS})

#
target_link_libraries(${bin} -Wl,--start-group stitch  detocr zh_frontend_lib g2p_en_lib openblas nncase.rt_modules.k230 Nncase.Runtime.Native functional_k230  pthread m sys  sentencepiece vo connector ai ao aenc adec vb freetype vicap cam_device cam_engine hal oslayer ebase fpga isp_drv binder auto_ctrol common cam_caldb isi 3a buffer_management cameric_drv video_in virtual_hal start_engine cmd_buffer switch cameric_reg_drv t_database_c t_mxml_c t_json_c t_common_c sensor -Wl,--end-group )

#install(TARGETS ${bin} DESTINATION out)








