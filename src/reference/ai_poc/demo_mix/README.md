# 1.简介

本项目实现了多个demo串烧操作，是手势关键点识别、动态手势识别、人脸姿态角和单目标跟踪的集成。可以作为智能跟踪拍摄车的软件部分实现隔空调整底盘位置，隔空调整相机角度，追踪人脸目标。

本项目还设计了数据帧的格式并实现了通过串口发送数据并提供了串口测试脚本test_com.py。

本项目仅支持在k230_sdk 1.3（包含）以前版本实现串口通信，后续版本因代码修改不再支持独立串口通信。请确认下载镜像版本正确。

# 2.应用使用说明

## 2.1使用帮助

### 2.1.1 手势说明

支持手势如下：

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/demo_mix/gesture.jpg" alt="手势关键点类别" width="50%" height="50%"/>

one手势进入动态手势识别，love手势退出；yeah手势进入姿态角调整，会选择距离屏幕中心点最近的点调整，love手势退出；three手势进入单目标追踪，会选择距离屏幕中心点最近的人脸进行跟踪。

### 2.1.2串口通信

使用CANMV-K230开发板进行串口通信的连接方式：

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/demo_mix/canmv_k230_serial.png" alt="串口通信连接图示" width="50%" height="50%"/>

串口连接后，除了大小核串口外，剩余的另一个串口就是数据串口。test_com.py中设置的串口为“COM25”，在运行时您可以根据您的串口编号修改。

协议格式如图所示：

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/demo_mix/data_frame.png" alt="协议格式图示" width="50%" height="50%"/>

接收到的十六进制数据两位为一个字节，前两位表示帧头AA，最后两位表示帧尾BB，第3，4位表示设备编号，0为底盘，1为相机；第5、6位表示命令编号，设备0有5类命令（0，1，2，3，4），设备1有2类命令（0，1）；第7、8位表示数据长度，单位是字节；剩余数据为数据。

注意：数据解析时使用int8格式范围为-128~127；

### 2.1.3 动态手势识别命令

| 指令（16进制） | 说明                                                         | 解释                 |
| -------------- | ------------------------------------------------------------ | -------------------- |
| AA00000100BB   | frame head:AA<br />device_num:00 <br />command_num:00  <br />data_length:01 <br />data:00<br />frame tail:BB | 底盘小车开始向前移动 |
| AA00010101BB   | frame head:AA<br />device_num:00 <br />command_num:01 <br />data_length:01 <br />data:01<br />frame tail:BB | 底盘小车开始向左移动 |
| AA00020102BB   | frame head:AA<br />device_num:00 <br />command_num:02 <br />data_length:01 <br />data:02<br />frame tail:BB | 底盘小车开始向后移动 |
| AA00030103BB   | frame head:AA<br />device_num:00 <br />command_num:03 <br />data_length:01 <br />data:03<br />frame tail:BB | 底盘小车开始向右移动 |
| AA00040104BB   | frame head:AA<br />device_num:00 <br />command_num:04 <br />data_length:01 <br />data:04<br />frame tail:BB | 底盘小车停止移动     |

接收到指令后会一直移动，直到收到middle命令才能继续下一个方向的调整，因此不同方向的调整命令必须由middle作为停止命令分开。

### 2.1.4 人脸姿态角命令

| 指令（16进制）   | 说明                                                         | 解释                                                         |
| ---------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| AA010003xxxxxxBB | frame head:AA<br />device_num:01 <br />command_num:00  <br />data_length:03 <br />data:xxxxxx<br />frame tail:BB | data数据每两位解析成一个int8数据（-128~127），分别表示与x、z、y轴的正方向的夹角 |

姿态角的y,z,x的示意图如下：

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/demo_mix/pose.png" alt="姿态角方向图示" width="50%" height="50%"/>

### 2.1.5 单目标人脸跟踪命令

| 指令（16进制） | 说明                                                         | 解释                                                         |
| -------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| AA0101020000BB | frame head:AA<br />device_num:01 <br />command_num:01  <br />data_length:02 <br />data:0000<br />frame tail:BB | data数据占4位，前两位表示左右偏向，后两位表示上下偏向；0000表示跟踪正常，停止小车移动和相机转动 |
| AA0101020100BB | frame head:AA<br />device_num:01 <br />command_num:01  <br />data_length:02 <br />data:0100<br />frame tail:BB | 0100，相机向左转动，直到出现0000停住                         |
| AA0101020200BB | frame head:AA<br />device_num:01 <br />command_num:01  <br />data_length:02 <br />data:0200<br />frame tail:BB | 0200，相机向右转动，直到出现0000停住                         |
| AA0101020001BB | frame head:AA<br />device_num:01 <br />command_num:01  <br />data_length:02 <br />data:0001<br />frame tail:BB | 0001，相机向下转动，直到出现0000停住                         |
| AA0101020002BB | frame head:AA<br />device_num:01 <br />command_num:01  <br />data_length:02 <br />data:0002<br />frame tail:BB | 0002，相机向上转动，直到出现0000停住                         |
| AA0101020101BB | frame head:AA<br />device_num:01 <br />command_num:01  <br />data_length:02 <br />data:0101<br />frame tail:BB | 0101，相机向左下转动，直到出现0000停住                       |
| AA0101020102BB | frame head:AA<br />device_num:01 <br />command_num:01  <br />data_length:02 <br />data:0102<br />frame tail:BB | 0102，相机向左上转动，直到出现0000停住                       |
| AA0101020201BB | frame head:AA<br />device_num:01 <br />command_num:01  <br />data_length:02 <br />data:0201<br />frame tail:BB | 0201，相机向右下转动，直到出现0000停住                       |
| AA0101020202BB | frame head:AA<br />device_num:01 <br />command_num:01  <br />data_length:02 <br />data:0202<br />frame tail:BB | 0202，相机向右上转动，直到出现0000停住                       |

### 2.1.6 快速启动

连接好串口，在PC端运行scripts目录下的test_com.py脚本，可以在命令行查看指令。

```shell
python test_com.py
```

无串口连接的情况下可以正常运行，不查看串口数据时可以不连串口。

在大核执行AI线程：

```shell
# 准备kmodel列表：hand_det.kmodel、handkp_det.kmodel、gesture.kmodel、face_detection_320.kmodel、face_pose.kmodel、cropped_test127.kmodel、nanotrack_backbone_sim.kmodel、nanotracker_head_calib_k230.kmodel
#准备文件：shang.bin、xia.bin、zuo.bin、you.bin、demo_mix.elf
#视频流推理（demo_mix.sh）
./demo_mix.elf
```

