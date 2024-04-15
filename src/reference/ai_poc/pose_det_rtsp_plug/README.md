# 1.简介

RTSP是实时流协议（Real-Time Streaming Protocol）的缩写。它是一种用于在计算机网络上控制实时数据流传输的网络协议。RTSP通常用于控制流媒体服务器上的音频或视频数据流的传输。

人体关键点检测是一项涉及识别图像或视频中人体对象特定点（通常称为关键点）位置的任务。关键点可以表示人体对象的各个部分，例如关节或其他独特特征。关键点的位置通常表示为一组 2D [x, y] 或 3D [x, y,visible] 坐标。人体关键点检测模型的输出是一组代表图像或视频中人体对象上的关键点，以及每个点的置信度得分。

本demo可以实现将k230开发板sensor读取的图像进行处理，绘制检测结果到图像上，将此图像通过rtsp推送到外部。

# 2.应用使用说明

## 2.1 使用帮助

注：编译可以执行文件前，需在src的上一级目录下执行 make cdk-user

大核命令提示：

```
Usage: ./pose_detect.elf<kmodel> <obj_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for isp] ./pose_detect.elf yolov8n-pose.kmodel 0.3 0.45 0
Options:
 1> kmodel          pose检测kmodel文件路径
 2> obj_thresh      pose检测阈值
 3> nms_thresh      NMS阈值
 4> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

小核命令提示：

```
Usage: ./rtspServer [-p phyAddr] [-t <codec_type>]
-p: phyAddr 大核指定的datafifo物理地址
-t: 视频编码类型: h264/h265, 默认h265
```

pc端安装VLC工具：https://www.videolan.org/vlc/index.zh_CN.html

##### 执行步骤：

1. 在大核执行rtsp_plug_big.sh

   ```
   ./pose_det_enc.elf yolov8n-pose.kmodel 0.3 0.45 0
   ```

2. 在小核执行rtsp_plug_little.sh（释放执行权限 chmod 777 rtsp_plug_little.sh）

   ```
   ./rtspServer -p 1628c000 -t h265
   ```

3. pc端打开VLC工具，拷贝小核输出ip到VLC，点击播放执行拉流操作（**确保开发板连接网线**）

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/pose_det_rtsp_plug/rtsp0.jpeg" alt="流程0" width="50%" height="50%" />

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/pose_det_rtsp_plug/rtsp1.jpeg" alt="流程1" width="50%" height="50%" />
   

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/pose_det_rtsp_plug/pose_det_rtsp_plug.jpg" alt="rtsp推流效果" width="50%" height="50%" />