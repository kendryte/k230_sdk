aux_source_directory(. DIR_ZH_FRONTEND_SRCS)
add_library(zh_frontend_lib STATIC ${DIR_ZH_FRONTEND_SRCS})

ADD_SUBDIRECTORY(cppjieba)
ADD_SUBDIRECTORY(pypinyin)
ADD_SUBDIRECTORY(zh_normalization)

target_link_libraries(zh_frontend_lib  pypinyin_lib zh_normalization_lib)