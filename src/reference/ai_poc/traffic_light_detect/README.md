# 1.简介

交通信号灯检测采用了yolov5网络结构。使用该应用，可检测到图像或视频中的交通信号灯。

# 2.应用使用说明

## 2.1 使用帮助

```
 Usage: ./traffic_light_detect.elf <kmodel> <tld_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for img] ./traffic_light_detect.elf traffic.kmodel 0.5 0.45 traffic.jpg 0
 [for isp] ./traffic_light_detect.elf traffic.kmodel 0.5 0.45 None 0
Options:
 1> kmodel    交通信号灯检测kmodel文件路径
 2> tld_thresh  交通信号灯检测阈值
 3> nms_thresh  NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```
