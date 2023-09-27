# 1.简介

健身动作计数采用了yolov8网络结构。使用该应用，可对视频中单人的健身动作（deep-down）进行计数。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./fitness.elf<kmodel> <obj_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for isp] ./fitness.elf yolov8n-pose.kmodel 0.1 0.45 None 0
Options:
 1> kmodel    pose检测kmodel文件路径
 2> obj_thresh  pose检测阈值
 3> nms_thresh  NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

