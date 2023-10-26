cmake_minimum_required(VERSION 3.2)

project(nncase_sdk C CXX)


set(nncase_sdk_root "${PROJECT_SOURCE_DIR}/../../big/nncase/")
set(k230_sdk ${nncase_sdk_root}/../../../)
set(CMAKE_EXE_LINKER_FLAGS "-T ${PROJECT_SOURCE_DIR}/cmake/link.lds --static")

# set opencv
set(k230_opencv ${k230_sdk}/src/big/utils/lib/opencv)
include_directories(${k230_opencv}/include/opencv4/)
link_directories(${k230_opencv}/lib  ${k230_opencv}/lib/opencv4/3rdparty)

# set mmz
link_directories(${k230_sdk}/src/big/mpp/userapps/lib)

# set nncase
include_directories(${nncase_sdk_root}/riscv64)
include_directories(${nncase_sdk_root}/riscv64/nncase/include)
include_directories(${nncase_sdk_root}/riscv64/nncase/include/nncase/runtime)
link_directories(${nncase_sdk_root}/riscv64/nncase/lib/)

add_subdirectory(./multimodal_chat_robot/src/multimodal_chat_robot_client/aiot_gpt)
add_subdirectory(./meta_human/src/meta_human)
add_subdirectory(./finger_reader/src/finger_reader_client/finger_reader_ai)
add_subdirectory(./meta_hand/src/meta_hand)
add_subdirectory(./housekeeper/src/housekeeper_client/housekeeper_ai/housekeeper)
add_subdirectory(./housekeeper/src/housekeeper_client/housekeeper_ai/face_registration)
add_subdirectory(./ai_scale/src/ai_scale_ai_code)


