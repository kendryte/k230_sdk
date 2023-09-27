set(LV_DRIVERS_CONF_DIR ${CMAKE_CURRENT_SOURCE_DIR})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} LV_DRIVERS_SOURCES)

add_library(lv_drivers SHARED ${LV_DRIVERS_SOURCES})
add_library(lvgl::drivers ALIAS lv_drivers)

target_include_directories(lv_drivers
    PRIVATE ${LV_DRIVERS_CONF_DIR})

target_link_libraries(lv_drivers
    PRIVATE lvgl
    PRIVATE disp
    PRIVATE pthread
    PRIVATE gcc)

install(TARGETS lv_drivers
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
