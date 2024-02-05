# 1.简介

人工智能浪潮席卷各行各业，智能运动健身行业应运而生。刘畊宏引领的《本草纲目》健身操开启了居家健身的新纪元。与之而来的是人们对智能健身的识别、矫正、计数的更高需求。本应用很好地满足了这些需求。

本应用采用了yolov8网络结构，对视频中健身动作（单人deep-down）进行计数操作。健身动作对边缘端设备的检测响应速度要求极高，本应用所部属的轻量化模型很好地满足速度要求。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./fitness.elf<kmodel> <obj_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for isp] ./fitness.elf yolov8n-pose.kmodel 0.3 0.45 None 0
Options:
 1> kmodel          pose检测kmodel文件路径
 2> obj_thresh      pose检测阈值
 3> nms_thresh      NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/fitness/fitness.gif" alt="健身动作识别计数效果图" width="50%" height="50%" />

以上效果图是在EVB板子上实现的。