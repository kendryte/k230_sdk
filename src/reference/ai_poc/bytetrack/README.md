# 1.简介

目标追踪采用了yolov5网络结构。使用该应用，可实时追踪视频中的每个人的轨迹。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./bytetrack.elf <kmodel> <pd_thresh> <nms_thresh> <input_mode> <debug_mode> <fps> <buffer>
For example:
 [for isp] ./bytetrack.elf bytetrack_yolov5n.kmodel 0.5 0.45 None 0 24 30
 [for img] ./bytetrack.elf bytetrack_yolov5n.kmodel 0.5 0.45 277 0 24 30
Options:
 1> kmodel    bytetrack行人检测kmodel文件路径
 2> pd_thresh  行人检测阈值
 3> nms_thresh  NMS阈值
 4> input_mode       图像 (Number) or 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 6> fps         帧率。对于img分支，fps最好设置为抽帧的fps。
 7> buffer      容忍帧数，即超过多少帧之后无法匹配上某个track，就认为该track丢失

```

[Attention] images in "/data/k230_ai_demo_nncase_2.1/ai_poc/images/bytetrack_data" 