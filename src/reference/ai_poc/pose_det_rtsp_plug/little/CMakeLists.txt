cmake_minimum_required(VERSION 3.2)
include(./cmake/Riscv64.cmake)

project(rtspServer C CXX)

set(nncase_sdk_root "${PROJECT_SOURCE_DIR}/../../../../big/nncase")
set(k230_sdk ${nncase_sdk_root}/../../../)
set(CDK_SRC_DIR ${k230_sdk}/src/common/cdk)
set(RTSMART_SRC_DIR ${k230_sdk}/src/big/rt-smart)
set(IPCMSG_ROOT ${CDK_SRC_DIR}/user/component/ipcmsg)
set(MAPI_ROOT ${CDK_SRC_DIR}/user/mapi)
set(MPI_ROOT ${RTSMART_SRC_DIR}/userapps/mpp)
set(LIVE_ROOT ${CDK_SRC_DIR}/user/thirdparty/live)
set(MW_ROOT ${CDK_SRC_DIR}/user/middleware)

set(rtsp_server_root "${PROJECT_SOURCE_DIR}/src")

set(CMAKE_INSTALL_PREFIX "../")
file(GLOB APP_SOURCES ${rtsp_server_root}/*.cpp)

set(BIN rtspServer)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${k230_sdk}/src/big/mpp/userapps/api/)
include_directories(${k230_sdk}/src/big/mpp/include)
include_directories(${k230_sdk}/src/big/mpp/include/comm)
include_directories(${k230_sdk}/src/common/cdk/user/component/datafifo/include)

include_directories(${IPCMSG_ROOT}/include)
include_directories(${MAPI_ROOT}/include)
include_directories(${MAPI_ROOT}/include/api)
include_directories(${MAPI_ROOT}/include/comm)
include_directories(/usr/local/include)
include_directories(${MW_ROOT}/rtsp_server/include)

link_directories(${k230_sdk}/src/common/cdk/user/component/datafifo/slave/lib)
link_directories(${k230_sdk}/src/big/mpp/userapps/lib)

link_directories(${IPCMSG_ROOT}/host/lib)
link_directories(${MAPI_ROOT}/mediaclient/lib)
link_directories(${MW_ROOT}/rtsp_server/lib)
link_directories(${LIVE_ROOT}/BasicUsageEnvironment)
link_directories(${LIVE_ROOT}/groupsock)
link_directories(${LIVE_ROOT}/liveMedia)
link_directories(${LIVE_ROOT}/UsageEnvironment)
link_directories(/usr/local/lib)


add_executable(${BIN} ${APP_SOURCES})
target_link_libraries(${BIN} -Wl,--start-group datafifo pthread sys ipcmsg mapi rtsp_server liveMedia UsageEnvironment BasicUsageEnvironment groupsock -Wl,--end-group)

install(TARGETS ${BIN} DESTINATION out)