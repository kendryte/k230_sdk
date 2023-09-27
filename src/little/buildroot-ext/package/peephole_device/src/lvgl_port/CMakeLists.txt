set(LVGL_ROOT_DIR "${CMAKE_SOURCE_DIR}/thirdlib/lvgl")
set(LVGL_PORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Option to define LV_LVGL_H_INCLUDE_SIMPLE, default: ON
option(LV_LVGL_H_INCLUDE_SIMPLE
    "Use #include \"lvgl.h\" instead of #include \"../../lvgl.h\"" ON)

# Option to define LV_CONF_INCLUDE_SIMPLE, default: ON
option(LV_CONF_INCLUDE_SIMPLE
    "Simple include of \"lv_conf.h\" and \"lv_drv_conf.h\"" ON)

# Option to define LV_INSTALL_INCLUDE, default: OFF
option(LV_INSTALL_INCLUDE
    "Install LVGL include files" OFF)

set(LVGL_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")

if(DEMO_BUILD)
    file(GLOB_RECURSE LVGL_DEMO_SOURCES
        ${LVGL_ROOT_DIR}/demos/*.c
        ${LVGL_ROOT_DIR}/examples/*.c)
    set(LVGL_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf_demo.h")
endif()

file(GLOB_RECURSE LVGL_SOURCES ${LVGL_ROOT_DIR}/src/*.c)

add_library(lvgl SHARED ${LVGL_SOURCES} ${LVGL_DEMO_SOURCES})
add_library(lvgl::lvgl ALIAS lvgl)

find_package(PkgConfig)
pkg_check_modules(PKG_FREETYPE
    freetype2)

target_compile_definitions(lvgl
    PUBLIC $<$<BOOL:${LV_LVGL_H_INCLUDE_SIMPLE}>:LV_LVGL_H_INCLUDE_SIMPLE>
    PUBLIC $<$<BOOL:${LV_CONF_INCLUDE_SIMPLE}>:LV_CONF_INCLUDE_SIMPLE>
    PUBLIC LV_CONF_PATH=${LVGL_CONF_PATH})

target_include_directories(lvgl
    PUBLIC ${LVGL_ROOT_DIR}
    PUBLIC ${LVGL_PORT_DIR}
    PRIVATE ${PKG_FREETYPE_INCLUDE_DIRS})

target_link_libraries(lvgl
    PRIVATE ${PKG_FREETYPE_LIBRARIES})

if(LV_INSTALL_INCLUDE)
    install(DIRECTORY
        "${LVGL_ROOT_DIR}/src"
        "${LVGL_ROOT_DIR}/demos"
        "${LVGL_ROOT_DIR}/examples"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/lvgl"
        FILES_MATCHING
        PATTERN "*.h")

    set(LVGL_PUBLIC_HEADERS
        "${LVGL_PORT_DIR}/lv_port.h"
        "${LVGL_ROOT_DIR}/lvgl.h")
endif()

set_target_properties(lvgl PROPERTIES
    OUTPUT_NAME lvgl
    PUBLIC_HEADER "${LVGL_PUBLIC_HEADERS}")

install(TARGETS lvgl
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/lvgl")

# build lv_drivers
if(NATIVE_BUILD)
    add_subdirectory(pc)
else()
    add_subdirectory(k230)
endif()
