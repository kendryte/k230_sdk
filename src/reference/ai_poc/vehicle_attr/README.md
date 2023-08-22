# 1.简介

车辆属性识别采用了yolov5网络结构、pulc属性识别网络。使用该应用，可检测到图像或视频中的每个车的车辆属性。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./vehicle_attr.elf<kmodel> <od_thresh> <nms_thresh> <input_mode> <attr_kmodel> <color_thresh> <type_thresh> <debug_mode>
For example:
 [for img] ./vehicle_attr.elf vehicle_attr_yolov5n.kmodel 0.1 0.45 car.jpg vehicle.kmodel 0.1 0.1 0
 [for isp] ./vehicle_attr.elf vehicle_attr_yolov5n.kmodel 0.1 0.45 None vehicle.kmodel 0.1 0.1 0
Options:
 1> kmodel    检测kmodel文件路径
 2> od_thresh  检测阈值
 3> nms_thresh  NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> attr_kmodel 属性识别kmodel文件路径
 6> color_thresh 颜色识别阈值
 7> type_thresh 车型识别阈值
 8> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

