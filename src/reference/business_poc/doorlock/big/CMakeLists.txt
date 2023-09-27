cmake_minimum_required(VERSION 3.2)
include(../../../../big/nncase/examples/cmake/Riscv64.cmake)

project(door_lock C CXX)
set(nncase_sdk_root "${PROJECT_SOURCE_DIR}/../../../../big/nncase")
set(k230_sdk ${nncase_sdk_root}/../../../)
set(CMAKE_EXE_LINKER_FLAGS "-T ${nncase_sdk_root}/examples/cmake/link.lds --static")
set(src main.cc model.cc mobile_retinaface.cc anchors_320.cc util.cc mobile_face.cc)
set(bin door_lock.elf)
set(script ./script/init.sh)
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../" CACHE STRING "The path to use for make install" FORCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp -march=rv64imafdcv -mabi=lp64d -mcmodel=medany" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set(k230_opencv ${k230_sdk}/src/big/utils/lib/opencv)
set(CMAKE_INSTALL_PREFIX "../")
include_directories(${k230_opencv}/include/opencv4/)
link_directories(${k230_opencv}/lib  ${k230_opencv}/lib/opencv4/3rdparty)

include_directories(${nncase_sdk_root}/riscv64)
include_directories(${nncase_sdk_root}/riscv64/nncase/include)
include_directories(${nncase_sdk_root}/riscv64/nncase/include/nncase/runtime)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${nncase_sdk_root}/riscv64/rvvlib/include)
include_directories(${k230_sdk}/src/big/mpp/userapps/api/)
include_directories(${k230_sdk}/src/big/mpp/include)
include_directories(${k230_sdk}/src/big/mpp/include/comm)
include_directories(${k230_sdk}/src/big/mpp/userapps/sample/sample_vo)
include_directories(${k230_sdk}/src/common/cdk/user/component/ipcmsg/include)
include_directories(${k230_sdk}/src/common/cdk/user/component/ipcmsg/sample)
include_directories(${k230_sdk}/src/rt-smart/kernel/rt-thread/libcpu/risc-v/virt64)


link_directories(${k230_sdk}/src/big/mpp/userapps/lib)
link_directories(${nncase_sdk_root}/riscv64/rvvlib/)
link_directories(${nncase_sdk_root}/riscv64/nncase/lib/)
link_directories(${k230_sdk}/src/common/cdk/user/component/ipcmsg/host/lib)

add_executable(${bin} ${src})
target_link_libraries(${bin} -Wl,--start-group rvv Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 sys vicap vb cam_device cam_engine
 hal oslayer ebase fpga isp_drv binder auto_ctrol common cam_caldb isi 3a buffer_management cameric_drv video_in virtual_hal start_engine cmd_buffer
 switch cameric_reg_drv t_database_c t_mxml_c t_json_c t_common_c vo sensor connector atomic ipcmsg -Wl,--end-group)

target_link_libraries(${bin} opencv_core opencv_imgcodecs opencv_imgproc opencv_highgui opencv_videoio opencv_core zlib libjpeg-turbo libopenjp2 libpng libtiff libwebp csi_cv)

install(TARGETS ${bin} DESTINATION out)
