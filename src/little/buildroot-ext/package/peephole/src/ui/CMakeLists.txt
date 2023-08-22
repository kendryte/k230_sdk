get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# build lvgl share library
add_subdirectory(lvgl_port)

# config exe_target
set(EXE_TARGET ${FOLDER_NAME})

aux_source_directory(src EXE_SRC)

add_executable(${EXE_TARGET}
    ${EXE_SRC})

set(IPCMSG_PATH $ENV{CDK_SRC_DIR}/user/component/ipcmsg)
set(MPP_PATH $ENV{MPP_SRC_DIR})
set(MAPI_PATH $ENV{CDK_SRC_DIR}/user/mapi)
set(MIDDLEWARE_PATH $ENV{CDK_SRC_DIR}/user/middleware)
set(DATAFIFO_PATH $ENV{CDK_SRC_DIR}/user/component/datafifo)
set(LIVE_PATH $ENV{CDK_SRC_DIR}/user/thirdparty/live)

target_include_directories(${EXE_TARGET}
    PRIVATE ${IPCMSG_PATH}/include
    PRIVATE ${MPP_PATH}/include
    PRIVATE ${MPP_PATH}/include/comm
    PRIVATE ${MPP_PATH}/userapps/api
    PRIVATE ${MAPI_PATH}/include
    PRIVATE ${MAPI_PATH}/include/comm
    PRIVATE ${MAPI_PATH}/include/api
    PRIVATE ${MIDDLEWARE_PATH}/rtsp_client/include
    PRIVATE ${MIDDLEWARE_PATH}/audio_mix/include
    PRIVATE ${MIDDLEWARE_PATH}/mp4_format/include
    PRIVATE ${DATAFIFO_PATH}/include)

target_link_directories(${EXE_TARGET}
    PRIVATE ${IPCMSG_PATH}/host/lib
    PRIVATE ${MAPI_PATH}/mediaclient/lib/
    PRIVATE ${MIDDLEWARE_PATH}/rtsp_client/lib
    PRIVATE ${MIDDLEWARE_PATH}/audio_mix/lib
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
    PRIVATE pthread
    PRIVATE ipcmsg
    PRIVATE mapi
    PRIVATE rtsp_client
    PRIVATE datafifo
    PRIVATE liveMedia
    PRIVATE BasicUsageEnvironment
    PRIVATE groupsock
    PRIVATE UsageEnvironment   
    PRIVATE audio_mix
    PRIVATE mp4)

target_compile_definitions(${EXE_TARGET}
    PRIVATE DATA_FILE_PATH="${CMAKE_INSTALL_PREFIX}/${FOLDER_NAME}/data/")

install(TARGETS ${EXE_TARGET}
    RUNTIME DESTINATION ${FOLDER_NAME})

install(DIRECTORY
    "${CMAKE_CURRENT_LIST_DIR}/data"
    DESTINATION ${FOLDER_NAME})

if(DEMO_BUILD)
    set(DEMO_TARGET "${FOLDER_NAME}-demo")

    aux_source_directory(demo DEMO_SRC)

    add_executable(${DEMO_TARGET}
        ${DEMO_SRC})

    target_link_libraries(${DEMO_TARGET}
        PRIVATE lvgl::lvgl
        PRIVATE lvgl::drivers)

    install(TARGETS ${DEMO_TARGET}
        RUNTIME DESTINATION ${FOLDER_NAME})
endif()
