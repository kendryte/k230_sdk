# 1.简介

安全帽是建筑业、制造业等工业生产中重要的劳保工具，应用十分广泛。近年来，因不佩戴安全帽、不规范佩戴安全帽等原因导致的安全生产事故屡禁不止，事故发生背后的影响是巨大的，不仅为家人带来巨大的伤痛，也为企业的利益带来巨大的损失。同时，由于企业的监督不到位，因未佩戴安全帽而引发的安全事故不计其数，因此对工作人员进行安全帽佩戴状况的实时检测是非常重要且必要的。

本应用采用了yolov5网络结构。使用该应用，可检测到图像或视频中人员是否佩戴安全帽。

# 2.应用使用说明

## 2.1 使用帮助

```
 Usage: ./helmet_detect.elf <kmodel> <od_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for img] ./helmet_detect.elf helmet.kmodel 0.5 0.45 helmet.jpg 0
 [for isp] ./helmet_detect.elf helmet.kmodel 0.5 0.45 None 0
Options:
 1> kmodel          头盔检测kmodel文件路径
 2> od_thresh       头盔检测阈值
 3> nms_thresh      NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/helmet_detect/helmet_detect.gif" alt="安全帽检测效果图" width="50%" height="50%" />