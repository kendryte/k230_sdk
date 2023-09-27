# 1.简介

抽烟检测采用了yolov5网络结构。使用该应用，可检测到图像或视频中是否有抽烟行为。

# 2.应用使用说明

## 2.1 使用帮助

```
 Usage: ./smoke_detect.elf <kmodel> <sd_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for img] ./smoke_detect.elf yolov5s_smoke_best.kmodel 0.5 0.45 smoke1.jpg 0
 [for isp] ./smoke_detect.elf yolov5s_smoke_best.kmodel 0.5 0.45 None 0
Options:
 1> kmodel    抽烟检测kmodel文件路径
 2> sd_thresh  抽烟检测阈值
 3> nms_thresh  NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```
