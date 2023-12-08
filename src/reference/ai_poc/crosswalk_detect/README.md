# 1.简介

人行横道检测是一种环境感知应用，其目的是通过车载相机或激光雷达来检测人行横道。近年来，随着计算机视觉的应用发展和落地，人行横道检测也获得了广泛关注，出现一系列的人行横道检测方法。人行横道检测在自动驾驶系统中扮演着重要的角色，特别是在高级辅助驾驶系统(ADAS)中，可以为驾驶系统提供决策依据。

本应用采用了yolov5网络结构。使用该应用，可检测到图像或视频中是否有人行横道。

# 2.应用使用说明

## 2.1 使用帮助

```
 Usage: ./crosswalk_detect.elf <kmodel> <cw_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for img] ./crosswalk_detect.elf crosswalk.kmodel 0.5 0.45 cw.jpg 0
 [for isp] ./crosswalk_detect.elf crosswalk.kmodel 0.5 0.45 None 0
Options:
 1> kmodel          人行横道检测kmodel文件路径
 2> cw_thresh       人行横道检测阈值
 3> nms_thresh      NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/crosswalk_detect/crosswalk_detect.gif" alt="人行横道检测效果图" width="50%" height="50%" />