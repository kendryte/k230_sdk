set(src main.cc ai_base.cc hifigan.cc fastspeech1.cc fastspeech2.cc VoxCommon.cpp length_regulator.cpp)
set(bin tts_zh.elf)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${nncase_sdk_root}/riscv64/rvvlib/include)
include_directories(${k230_sdk}/src/big/mpp/userapps/api/)
include_directories(${k230_sdk}/src/big/mpp/include)
include_directories(${k230_sdk}/src/big/mpp/include/comm)
link_directories(${nncase_sdk_root}/riscv64/rvvlib/)


add_executable(${bin} ${src})
target_link_libraries(${bin} -Wl,--start-group rvv Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 sys ao ai -Wl,--end-group)

target_link_libraries(${bin} zh_frontend_lib)
install(TARGETS ${bin} DESTINATION bin)