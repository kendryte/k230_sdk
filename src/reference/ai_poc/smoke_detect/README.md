# 1.简介

抽烟检测是指通过计算机视觉技术对人体抽烟行为进行实时监测和识别的过程。抽烟检测主要通过分析监控视频图像中人体的姿态、动作等特征，结合机器学习、深度学习等方法实现对抽烟行为的自动检测和识别。抽烟检测可以广泛应用于公共场所、办公场所、医院等场所的抽烟行为监管，有助于减少抽烟对健康的危害和减少抽烟对公共环境的污染。同时，抽烟检测也可以用于烟草研究、抽烟行为数据分析等领域。

本应用采用了yolov5网络结构。使用该应用，可检测到图像或视频中是否有抽烟行为。

# 2.应用使用说明

## 2.1 使用帮助

```
 Usage: ./smoke_detect.elf <kmodel> <sd_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for img] ./smoke_detect.elf yolov5s_smoke_best.kmodel 0.5 0.45 smoke1.jpg 0
 [for isp] ./smoke_detect.elf yolov5s_smoke_best.kmodel 0.2 0.45 None 0
Options:
 1> kmodel       抽烟检测kmodel文件路径
 2> sd_thresh    抽烟检测阈值
 3> nms_thresh   NMS阈值
 4> input_mode   本地图片(图片路径)/ 摄像头(None)
 5> debug_mode   是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/smoke_detect/sd_result.jpg" alt="抽烟检测效果图" width="50%" height="50%" />
