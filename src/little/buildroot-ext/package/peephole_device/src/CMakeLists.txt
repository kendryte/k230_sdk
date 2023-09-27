cmake_minimum_required(VERSION 3.10)
project(peephole_device C CXX)

set(CMAKE_INSTALL_PREFIX "/app")
set(CMAKE_INSTALL_LIBDIR "/usr/lib")
set(CMAKE_INSTALL_BINDIR "/usr/bin")


get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# build lvgl share library
add_subdirectory(lvgl_port)

# config exe_target
set(EXE_TARGET peephole_device)

aux_source_directory(src EXE_SRC)
aux_source_directory(src/assets EXE_SRC)
aux_source_directory(src/comm/src EXE_SRC)

add_executable(${EXE_TARGET}
    ${EXE_SRC})

set(IPCMSG_PATH $ENV{CDK_SRC_DIR}/user/component/ipcmsg)
set(MPP_PATH $ENV{MPP_SRC_DIR})
set(MAPI_PATH $ENV{CDK_SRC_DIR}/user/mapi)
set(MIDDLEWARE_PATH $ENV{CDK_SRC_DIR}/user/middleware)
set(DATAFIFO_PATH $ENV{CDK_SRC_DIR}/user/component/datafifo)
set(LIVE_PATH $ENV{CDK_SRC_DIR}/user/thirdparty/live)
set(THIRDPARTY_PATH $ENV{CDK_SRC_DIR}/user/thirdparty)

set(CMAKE_CXX_FLAGS "-std=c++14  -fPIC -O2  -DENABLE_SSL=OFF -DMSGPACK_NO_BOOST -DASIO_STANDALONE -Dunix ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "-fPIC -O2  -DENABLE_SSL=OFF -DMSGPACK_NO_BOOST -DASIO_STANDALONE -Dunix ${CMAKE_C_FLAGS}")

target_include_directories(${EXE_TARGET}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/comm/include
    PRIVATE ${IPCMSG_PATH}/include
    PRIVATE ${MPP_PATH}/include
    PRIVATE ${MPP_PATH}/include/comm
    PRIVATE ${MPP_PATH}/userapps/api
    PRIVATE ${MAPI_PATH}/include
    PRIVATE ${MAPI_PATH}/include/comm
    PRIVATE ${MAPI_PATH}/include/api
    PRIVATE ${MIDDLEWARE_PATH}/rtsp_server/include
    PRIVATE ${MIDDLEWARE_PATH}/audio_mix/include
    PRIVATE ${MIDDLEWARE_PATH}/kplayer/include
    PRIVATE ${MIDDLEWARE_PATH}/mp4_format/include
    PRIVATE ${THIRDPARTY_PATH}/asio
    PRIVATE ${THIRDPARTY_PATH}/msgpack/include
    PRIVATE ${THIRDPARTY_PATH}/rpc
    PRIVATE ${DATAFIFO_PATH}/include)

target_link_directories(${EXE_TARGET}
    PRIVATE ${IPCMSG_PATH}/host/lib
    PRIVATE ${MAPI_PATH}/mediaclient/lib/
    PRIVATE ${MIDDLEWARE_PATH}/rtsp_server/lib
    PRIVATE ${MIDDLEWARE_PATH}/audio_mix/lib
    PRIVATE ${MIDDLEWARE_PATH}/kplayer/lib
    PRIVATE ${MIDDLEWARE_PATH}/mp4_format/lib
    PRIVATE ${DATAFIFO_PATH}/slave/lib
    PRIVATE ${LIVE_PATH}
    PRIVATE ${LIVE_PATH}/BasicUsageEnvironment
    PRIVATE ${LIVE_PATH}/BasicUsageEnvironment
    PRIVATE ${LIVE_PATH}/UsageEnvironment
    PRIVATE ${LIVE_PATH}/groupsock
    PRIVATE ${LIVE_PATH}/liveMedia
    )

target_link_libraries(${EXE_TARGET}
    PRIVATE lvgl::lvgl
    PRIVATE lvgl::drivers
    PRIVATE kplayer
    PRIVATE mapi
    PRIVATE ipcmsg
    PRIVATE datafifo
    PRIVATE rtsp_server
    PRIVATE liveMedia
    PRIVATE BasicUsageEnvironment
    PRIVATE groupsock
    PRIVATE UsageEnvironment
    PRIVATE audio_mix
    PRIVATE mp4
    PRIVATE pthread)

target_compile_definitions(${EXE_TARGET}
    PRIVATE DATA_FILE_PATH="${CMAKE_INSTALL_PREFIX}/${FOLDER_NAME}/data/")

install(TARGETS ${EXE_TARGET}
    RUNTIME DESTINATION ${FOLDER_NAME})

install(DIRECTORY
    "${CMAKE_CURRENT_LIST_DIR}/data"
    DESTINATION ${FOLDER_NAME})

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
