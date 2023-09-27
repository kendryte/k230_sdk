## K230 AI Demo编译说明

K230 AI Demo集成了人脸、人体、手部、车牌、单词续写等模块，包含了分类、检测、分割、识别和跟踪等多种功能，给客户提供如何使用K230开发AI相关应用的参考。

K230 AI Demo编译支持CANMV-K230（右侧）和EVB-K230（左侧）2种开发板，编译时默认适配CANMV-K230开发板。

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/canmv_evb.jpg" alt="CANMV-K230 and EVB-K230对比图" width="50%" height="50%"/>

```cmake
#CMakeLists.txt
.....
include_directories(${nncase_sdk_root}/riscv64/nncase/include/nncase/runtime)
link_directories(${nncase_sdk_root}/riscv64/nncase/lib/)
#若需适配EVB-K230开发板，则去掉当前目录CMakeLists.txt中的add_definitions(-DCONFIG_BOARD_K230_CANMV)行
add_definitions(-DCONFIG_BOARD_K230_CANMV)

add_subdirectory(face_detection)
.....
```





