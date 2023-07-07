# 1.简介

摔倒检测采用了yolov5网络结构。使用该应用，可检测到图像或视频中摔倒的人。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./falldown_detect.elf<kmodel> <fdd_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for img] ./falldown_detect.elf yolov5n-falldown.kmodel 0.5 0.45 falldown_elder.jpg 0
 [for isp] ./falldown_detect.elf yolov5n-falldown.kmodel 0.3 0.45 None 0
Options:
 1> kmodel    摔倒检测kmodel文件路径
 2> fdd_thresh  摔倒检测阈值
 3> nms_thresh  NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

