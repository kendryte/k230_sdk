# cmake 最低版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 3.5.0)


# 设置项目工程名
PROJECT(${TARGET} VERSION 1.1.0)


# 构建时输出打印消息
MESSAGE("This is HelloWorld project.")


# 系统配置
enable_language(C ASM)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR cortex-a)


# 判断 RTT_EXEC_PATH 变量是否存在
if(NOT DEFINED ENV{RTT_EXEC_PATH})
    message(FATAL_ERROR "not defined environment variable: RTT_EXEC_PATH")
    message(FATAL_ERROR "Please execute the command: $ source art_pi_smart_vscode/smart-env.sh")
endif()



# 配置编译工具、参数
set(CONFIG_PREFIX "$ENV{RTT_EXEC_PATH}/arm-linux-musleabi-")

set(CMAKE_C_COMPILER "${CONFIG_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${CONFIG_PREFIX}g++")
set(CMAKE_ASM_COMPILER "${CONFIG_PREFIX}gcc")
set(CMAKE_OBJCOPY "${CONFIG_PREFIX}objcopy")
set(CMAKE_C_AR "${CONFIG_PREFIX}ar")
set(CMAKE_SIZE "${CONFIG_PREFIX}size")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a -marm -msoft-float -Werror -Wall -O0 -g -gdwarf-2 -n --static")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -march=armv7-a -marm -msoft-float -x assembler-with-cpp -O0 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a -marm -msoft-float -Werror -Wall -Woverloaded-virtual -fno-exceptions -fno-rtti -O0 -g -gdwarf-2 -n --static")


# userapps 根路径
set(UROOT_DIR "${CMAKE_SOURCE_DIR}/../..")

# rt-thread 路径
set(RT_DIR "${UROOT_DIR}/sdk/rt-thread")
set(INC_DIR "${UROOT_DIR}/sdk/rt-thread/include")
set(LIB_DIR "${UROOT_DIR}/sdk/rt-thread/lib")


# 设置 sdk 头文件
set(SDK_INC 
    "${RT_DIR}/components/dfs"
    "${RT_DIR}/components/drivers"
    "${RT_DIR}/components/finsh"
    "${RT_DIR}/components/net"
    "${RT_DIR}/components/net/netdev"
    "${RT_DIR}/components/net/arpa"

    "${INC_DIR}"
    "${INC_DIR}/libc"  
    "${INC_DIR}/sys"  
)


# 设置 app 头文件
set(APPS_INC 
    "${UROOT_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${SDK_INC}"
)


# 设置源文件
set(APPS_SRC 
    "${CMAKE_SOURCE_DIR}/main.c"
)


# 设置静态库
LINK_DIRECTORIES(${LIB_DIR}/librtthread.a)


# 设置链接文件
set(LINK_SCRIPTS_DIR "${UROOT_DIR}/linker_scripts/arm/cortex-a")
set(CMAKE_EXE_LINKER_FLAGS  "-T ${LINK_SCRIPTS_DIR}/link.lds -static")


# 设置可执行程序后缀名
SET(CMAKE_EXECUTABLE_SUFFIX ".elf")

# 生成可执行文件
ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${SDK_SRC}  ${APPS_SRC})


# 添加引用路径
target_include_directories(${PROJECT_NAME} PRIVATE ${APPS_INC})
