cmake_minimum_required(VERSION 3.10)
include(../../../../nncase/examples/cmake/Riscv64.cmake)

set(CMAKE_BUILD_TYPE Release)

project(sample-lvgl)
set(nncase_sdk_root "${PROJECT_SOURCE_DIR}/../../../../nncase")
set(k230_sdk ${nncase_sdk_root}/../../../)
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../" CACHE STRING "The path to use for make install" FORCE)

set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/../linker_scripts/riscv64/link.lds --static")
add_definitions(-march=rv64imafdcv -mcmodel=medany -mabi=lp64d -n --static)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../api
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/comm
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lvgl
)
link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib
)
add_subdirectory(../../src/lvgl ${CMAKE_CURRENT_BINARY_DIR}/lvgl)

add_executable(${PROJECT_NAME}.elf main.c)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    #target_link_options(${PROJECT_NAME}.elf PRIVATE -mcpu=c908v)
    if( supported )
        message(STATUS "IPO / LTO enabled")
        set_property(TARGET ${PROJECT_NAME}.elf PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_property(TARGET lvgl PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO / LTO not supported: <${error}>")
    endif()
endif()

target_link_libraries(${PROJECT_NAME}.elf lvgl lvgl::examples lvgl::demos lvgl::thorvg vg_lite vg_lite_util m vo connector vb sys)
install(TARGETS ${PROJECT_NAME}.elf DESTINATION elf)
