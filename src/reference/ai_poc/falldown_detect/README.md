# 1.简介

跌倒检测的应用场景丰富，比如餐饮场所（醉酒的人）、居家疗养场所（老人或儿童、患有心脏病或中风等疾病的人）、公共场所（游乐园的孩子、商场的行人）。跌倒检测可以对这些场景提供帮助。

本应用可以部署在终端设备（k230）进行摔倒行为检测，实现了多方向、多姿态、多种光照情况下的实时跌倒检测。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./falldown_detect.elf<kmodel> <fdd_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for img] ./falldown_detect.elf yolov5n-falldown.kmodel 0.5 0.45 falldown_elder.jpg 0
 [for isp] ./falldown_detect.elf yolov5n-falldown.kmodel 0.3 0.45 None 0
Options:
 1> kmodel          摔倒检测kmodel文件路径
 2> fdd_thresh      摔倒检测阈值
 3> nms_thresh      NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/falldown_detect/fdd_result.jpg" alt="跌倒检测效果图" width="50%" height="50%" />