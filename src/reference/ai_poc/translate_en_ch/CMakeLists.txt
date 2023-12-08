set(src ai_base.cc main.cc etranslate.cc dtranslate.cc)
set(bin translate_en_ch.elf)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${nncase_sdk_root}/riscv64/rvvlib/include)

link_directories(${PROJECT_SOURCE_DIR}/../../big/kmodel/ai_poc/utils/)
link_directories(${nncase_sdk_root}/riscv64/rvvlib/)

add_executable(${bin} ${src})
target_link_libraries(${bin} sentencepiece.a)
target_link_libraries(${bin} -Wl,--start-group rvv Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 sys)

install(TARGETS ${bin} DESTINATION bin)
