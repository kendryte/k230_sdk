# 1.简介

行人检测采用了yolov5网络结构。使用该应用，可检测到图像或视频中的每个行人。

# 2.应用使用说明

## 2.1 使用帮助

```
 Usage: ./person_detect.elf <kmodel> <pd_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for img] ./person_detect.elf person_detect_yolov5n.kmodel 0.5 0.45 bus.jpg 0
 [for isp] ./person_detect.elf person_detect_yolov5n.kmodel 0.5 0.45 None 0
Options:
 1> kmodel    行人检测kmodel文件路径
 2> pd_thresh  行人检测阈值
 3> nms_thresh  NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```
