
add_library(Binc
        adapter.c
        advertisement.c
        agent.c
        application.c
        characteristic.c
        descriptor.c
        device.c
        logger.c
        parser.c
        service.c
        utility.c
        )

target_include_directories (Binc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Binc ${GLIB_LIBRARIES} m)