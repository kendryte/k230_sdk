# 1.简介

行人检测( Pedestrian Detection)是利用计算机视觉技术判断图像或者视频序列中是否存在行人并给予精确定位。该技术可与行人跟踪，行人重识别等技术结合，应用于人工智能系统、车辆辅助驾驶系统、智能机器人、智能视频监控、人体行为分析、智能交通等领域。

由于行人兼具刚性和柔性物体的特性 ，外观易受穿着、尺度、遮挡、姿态和视角等影响，使得行人检测成为计算机视觉领域中一个既具有研究价值同时又极具挑战性的热门课题。

本应用采用了yolov5网络结构。使用该应用，可检测到图像或视频中的每个行人，返回每个行人的位置坐标。

# 2.应用使用说明

## 2.1 使用帮助

```
 Usage: ./person_detect.elf <kmodel> <pd_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for img] ./person_detect.elf person_detect_yolov5n.kmodel 0.5 0.45 bus.jpg 0
 [for isp] ./person_detect.elf person_detect_yolov5n.kmodel 0.5 0.45 None 0
Options:
 1> kmodel          行人检测kmodel文件路径
 2> pd_thresh       行人检测阈值
 3> nms_thresh      NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/person_detect/pd_result.jpg" alt="行人检测效果图" width="50%" height="50%" />