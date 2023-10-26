# K230 模拟AI电子秤
![Alt text](resource/image.png)
Document version: V1.0 Release date: 2023-10-7

## 介绍
该项目为K230 模拟AI电子秤开源工程。本工程基于K230应用了度量学习技术实现商品识别。本应用使用PC作为服务端控制上秤、下秤和展示商品；使用K230作为客户端建立对比向量库，然后捕捉图像进行向量相似度对比获得商品类别，将识别结果传给服务端展示给用户。得到商品类别后可以结合商品单价和商品重量得到价签。

<img src="./resource/frame.jpg" alt="frame" style="zoom: 50%;" />

本项目主要侧重于K230端功能的实现，使用PC端的“上秤”，“下秤”两个按钮模拟传感器的逻辑。

## 主要特点

- 自学习：无需重新训练模型，在底库数据集中添加对应类别，推理时即可识别；


- 小样本分类效果良好：每个类别只需要几张图片即可完成分类；

## 环境准备

    #######download.sh########
    for file in AIScale.zip;  
    do  
    wget https://ai.b-bug.org/k230/downloads/fancy_poc/ai_scale/$file || wget https://kendryte-download.canaan-creative.com/k230/downloads/fancy_poc/ai_scale/$file;  
    done  
    
    for file in onboard_v2.4.zip
    do
    wget https://ai.b-bug.org/k230/downloads/fancy_poc/ai_scale/k230_board/$file || wget https://kendryte-download.canaan-creative.com/k230/downloads/fancy_poc/ai_scale/k230_board/$file;  
    done

## 数据准备

待分类的数据需要按照固定格式进行组织，数据组织形式如下：

```
|-gallery
	|-0
	   |-0.jpg
	   |-1.jpg
	   |-2.jpg
	   |-...
	   |-label.txt
	|-1
	   |-0.jpg
	   |-1.jpg
	   |-...
	   |label.txt
	|-2
	   |-...
	|...
```

数据集的根目录为gallery，gallery包含若干个子文件夹，每个文件夹以类别编号命名，从0开始；类别文件夹下包括若干张图片和一个label.txt文件；label.txt文件中存储类别名称。该数据集用于在商品识别之前创建底库。

## 服务端准备

### 1.准备服务端运行环境

    #解压AIScale.zip即可


## 客户端准备

### 1. 源码编译

在k230 docker中在src/reference/ai_poc目录下执行./build_app.sh，得到编译后的 ai_scale.elf以及client

### 2.在k230上创建ai_scale工程

    ## 进入大小核共享文件夹/sharefs
    cd /sharefs
    mkdir ai_scale
    ##将需要的文件拷贝到ai_scale目录下，如编译的ai_scale.elf、client、对应的kmodel、底库数据gallery等文件

## 程序运行

### 1. 服务端：

    # 解压AIScale.zip
    cd AIScale/AIScale_v1.0
    start AIScale.exe #或双击AIScale.exe启动

**注意：**服务端界面启动后点击**启动**按钮， 服务端服务IP为运行服务器的本机IP，连接端口默认为8080，连接端口可自定义。

### 2. K230客户端

服务端启动后，会在日志框中显示待连接的ip和端口，小核连接时

    cd /sharefs/ai_scale
    #小核下执行（IP和端口由服务端设置）
    ./connect.sh 192.168.1.2 8080
    #小核下修改ai_scale_isp.sh文件的可执行文件名称
    vi ai_scale_isp.sh
    #将main.elf换为ai_scale.elf
    #大核下执行（./ai_scale_isp.sh）
    ./ai_scale.elf recognition.kmodel None gallery 5 1
    #gallery是底库数据集的路径，根据您自定义的数据进行输入

**注意：** 1、使用./connect.sh命令时需要使用服务端显示的相应IP地址及通信端口；

​	     2、PC和开发板IP需在同一网段下；

​	     3、启动时，先启动服务端，再启动小核程序，最后启动大核程序；结束时，先终止大核程序，再停止小核程序，最后关闭服务端界面；



## 结果展示

### 测试说明图

![测试说明](./resource/state.jpg)

PC启动服务端，开发板摄像头对准识别商品，调整开发板使得摄像头拍摄到的图像尽可能清晰。然后建立PC和开发板之间的连接，开始上秤识别。

### 服务端展示界面

![ai_scale_server](./resource/server.jpg)


### k230屏幕显示和服务器端显示

![show](./resource/show.gif)

- 称重重量由传感器获取，商品单价可以存储在PC端的数据库中，也可存储在开发板上，在构建底库时同时构建商品价格字典；

- 通过识别获得商品类别后，后续可以根据称重重量和单价计算当前商品的价格；
- 本项目侧重实现K230端的商品识别；
