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

target_include_directories(${EXE_TARGET}
    PRIVATE ${IPCMSG_PATH}/include
    PRIVATE ${MPP_PATH}/include)

target_link_directories(${EXE_TARGET}
    PRIVATE ${IPCMSG_PATH}/host/lib)

target_link_libraries(${EXE_TARGET}
    PRIVATE lvgl::lvgl
    PRIVATE lvgl::drivers
    PRIVATE pthread
    PRIVATE ipcmsg)

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
