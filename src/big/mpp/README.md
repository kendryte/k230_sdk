# mpp

## 概述

mpp(Media Process Platform 媒体处理平台)是嘉楠科技开发的一整套适配K230的多媒体组件，包含内核的驱动库，用户态的API库及各样的sample程序。

## 编译

### 编译准备

在mpp目录下配置环境变量`source build_env.sh`之后即可对mpp进行编译了

**注意：编译前请确保mpp在sdk目录的正确位置中。**

### 编译详细

- 编译全部的kernel层代码

  进入目录 `k230_sdk/src/src/big/mpp/kerne`

  运行`make`命令


- 单独编译某个kernel驱动库
  
  进入某个驱动目录例如 `k230_sdk/src/big/mpp/kernel/fft`

  运行`make`命令

- 编译全部的user层api代码

  进入目录 `k230_sdk/src/big/mpp/userapps/src`

  运行`make`命令


- 单独编译某个user层api库
  
  进入某个api源码目录例如 `k230_sdk/src/big/mpp/userapps/src/sensor`

  运行`make`命令

- 编译全部的user层sample代码

  进入目录 `k230_sdk/src/big/mpp/userapps/sample`

  运行`make`命令


- 单独编译某个user层sample
  
  进入某个sample目录例如 `k230_sdk/src/big/mpp/userapps/sample/sample_fft`

  运行`make`命令

