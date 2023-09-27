# 1.简介

ByteTrack是一种基于目标检测bbox的多目标追踪算法。该多目标追踪算法应用卡尔曼滤波算法进行边界框预测，应用匈牙利算法进行目标和轨迹间的匹配。本应用采用yolov5网络结构作为目标检测算法。ByteTrack算法的最大创新点就是对低分框的“二次匹配”，最大程度优化了追踪过程ID-Switch问题。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./bytetrack.elf <kmodel> <pd_thresh> <nms_thresh> <input_mode> <debug_mode> <fps> <buffer>
For example:
 [for isp] ./bytetrack.elf bytetrack_yolov5n.kmodel 0.5 0.45 None 0 24 30
 [for img] ./bytetrack.elf bytetrack_yolov5n.kmodel 0.5 0.45 277 0 24 30
Options:
 1> kmodel         bytetrack行人检测kmodel文件路径
 2> pd_thresh      行人检测阈值
 3> nms_thresh     NMS阈值
 4> input_mode     图像 (Number) or 摄像头(None)
 5> debug_mode     是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 6> fps            帧率。对于img分支，fps最好设置为抽帧的fps。
 7> buffer         容忍帧数，即超过多少帧之后无法匹配上某个track，就认为该track丢失

```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/bytetrack/150.jpg" alt="多目标跟踪效果图" width="50%" height="50%" />

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/bytetrack/188.jpg" alt="多目标跟踪效果图" width="50%" height="50%" />

