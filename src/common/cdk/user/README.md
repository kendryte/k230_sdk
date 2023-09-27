# cdk-user

## 概述

cdk (canaan-developmemt-kit)是嘉楠科技开发的一套基于MPI和核间通讯搭建的适配K230的多媒体组件，cdk-user是其中的用户层部分，它包含如下内容

- component 核间通讯组件包括IPCMSG和DATAFIFO
- middleware 多媒体中间件，例如rtsp server，mp4播放器等
- samples 集成类的demo
- thirdparty 第三方组件，例如live555

## 编译

### 编译准备

进入目录`k230_sdk/src/common/cdk/user`,配置环境变量 `source build_env.sh`

注意需确保cdk目录在k230_sdk的正确位置

### 编译详细

- 编译核间通讯库和sample代码

  进入目录 `k230_sdk/src/common/cdk/user/component/ipcmsg`

  执行`make`命令编译

- 编译mapi全部库和sample代码

  进入目录 `k230_sdk/src/common/cdk/user/mapi`

  执行`make`命令编译

- 编译某个mapi sample程序

  例如进入目录 `k230_sdk/src/common/cdk/user/mapi/sample/rt-smart`

  执行`make`命令编译

- 编译mapi server端库

  进入目录 `k230_sdk/src/common/cdk/user/mapi/mediaserver`

  执行`make`命令编译

- 编译mapi client端库

  进入目录 `k230_sdk/src/common/cdk/user/mapi/mediaclient`

  执行`make`命令编译

- 编译middleware中间件

  进入目录 `k230_sdk/src/common/cdk/user/middleware`

  执行`make`命令编译

- 编译cdk user sample 程序

  进入目录 `k230_sdk/src/common/cdk/user/samples`

  执行`make`命令编译

- 编译第三方组件

  进入目录 `k230_sdk/src/common/cdk/user/thirdparty`

  执行`make`命令编译
