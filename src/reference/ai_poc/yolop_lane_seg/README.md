# 1.简介

车道线检测是一种环境感知应用，其目的是通过车载相机或激光雷达来检测车道线。近年来，随着计算机视觉的应用发展和落地，车道线检测任务也获得了广泛关注，出现一系列的车道线检测方法。车道检测在自动驾驶系统中扮演着重要的角色，特别是在高级辅助驾驶系统(ADAS)中。

本应用采用了 yolop 网络结构。使用该应用，可在图像或视频中实现路面分割，即检测到车道线和可行驶区域，并加以颜色区分。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./yolop.elf <kmodel> <input_mode> <debug_mode>
For example:
 [for img] ./yolop.elf yolop.kmodel road.jpg 0
 [for isp] ./yolop.elf yolop.kmodel None 0
Options:
 1> kmodel          kmodel文件路径
 2> input_mode      本地图片(图片路径)/ 摄像头(None)
 3> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/yolop_lane_seg/seg_result.jpg" alt="车道线检测效果图" width="50%" height="50%" />