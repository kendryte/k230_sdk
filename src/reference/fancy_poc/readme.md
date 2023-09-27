## K230 Fancy Poc编译说明

K230 Fancy Poc项目中包含了多模态聊天机器人、指读、虚拟数字人和虚拟数字手4个Poc工程，用户可以参考本项目进行Poc开发。

K230 Fancy Poc编译支持EVB-K230（左侧）和CANMV-K230（右侧）2种开发板，编译时默认适配CANMV-K230开发板。

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/canmv_evb.jpg" alt="CANMV-K230 and EVB-K230对比图" width="50%" height="50%"/>

```cmake
#CMakeLists.txt
.....
include_directories(${nncase_sdk_root}/riscv64/nncase/include/nncase/runtime)
link_directories(${nncase_sdk_root}/riscv64/nncase/lib/)
#若需适配EVB-K230开发板，则去掉当前目录CMakeLists.txt中的add_definitions(-DCONFIG_BOARD_K230_CANMV)行
add_definitions(-DCONFIG_BOARD_K230_CANMV)

add_subdirectory(./multimodal_chat_robot/src/multimodal_chat_robot_client/aiot_gpt)
add_subdirectory(./meta_human/src/meta_human)
add_subdirectory(./finger_reader/src/finger_reader_client/finger_reader_ai)
add_subdirectory(./meta_hand/src/meta_hand)
```





