set(src main.cc utils.cc ai_base.cc face_detection.cc face_glasses.cc anchors_320.cc anchors_640.cc)
set(bin face_glasses.elf)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${nncase_sdk_root}/riscv64/rvvlib/include)
include_directories(${k230_sdk}/src/big/mpp/userapps/api/)
include_directories(${k230_sdk}/src/big/mpp/include)
include_directories(${k230_sdk}/src/big/mpp/include/comm)
include_directories(${k230_sdk}/src/big/mpp/userapps/sample/sample_vo)
link_directories(${nncase_sdk_root}/riscv64/rvvlib/)

add_executable(${bin} ${src})
target_link_libraries(${bin} nncase.rt_modules.k230 Nncase.Runtime.Native functional_k230 sys)
target_link_libraries(${bin} rvv vicap vb cam_device cam_engine
 hal oslayer ebase fpga isp_drv binder auto_ctrol common cam_caldb isi 3a ahdr buffer_management avs cameric_drv
 aflt adci aca aee awdr3 cameric_reg_drv t_database_c t_shell_c t_mxml_c t_json_c t_common_c vo sensor)

target_link_libraries(${bin} opencv_imgproc opencv_imgcodecs opencv_core zlib libjpeg-turbo libopenjp2 libpng libtiff libwebp csi_cv)
install(TARGETS ${bin} DESTINATION bin)