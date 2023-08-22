project(socket_gpt_image_audio)
set(CMAKE_C_COMPILER "/opt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin/riscv64-unknown-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "/opt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin/riscv64-unknown-linux-gnu-g++")
set(src main.cc)
set(bin socket_gpt_image_audio)

add_executable(${bin} ${src})

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

install(TARGETS ${bin} DESTINATION ${INSTALL_DIR})

