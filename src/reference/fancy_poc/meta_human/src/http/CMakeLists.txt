cmake_minimum_required(VERSION 3.2)
include(./cmake/Riscv64.cmake)

project(http C CXX)

set(nncase_sdk_root "${PROJECT_SOURCE_DIR}/../../../../../big/nncase")
set(k230_sdk ${nncase_sdk_root}/../../../)
set(http_root "${PROJECT_SOURCE_DIR}/http")
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../../../" CACHE STRING "The path to use for make install" FORCE)
set(CMAKE_INSTALL_PREFIX "../../../")
file(GLOB APP_SOURCES ${http_root}/*.c)

set(BIN http)

include_directories(${k230_sdk}/src/big/mpp/userapps/api/)
include_directories(${k230_sdk}/src/big/mpp/include)
include_directories(${k230_sdk}/src/big/mpp/include/comm)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/http)

link_directories(${PROJECT_SOURCE_DIR}/slave/lib)
link_directories(${k230_sdk}/src/big/mpp/userapps/lib)

add_executable(${BIN} ${APP_SOURCES})
target_link_libraries(${BIN} -Wl,--start-group datafifo pthread sys -Wl,--end-group)

install(TARGETS ${BIN} DESTINATION ${PROJECT_SOURCE_DIR}/out)