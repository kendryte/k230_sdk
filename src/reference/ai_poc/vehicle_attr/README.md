# 1.简介

车辆属性识别可以分为两个任务：车辆检测和多标签分类。本应用采用了yolov5网络结构实现了车辆检测功能，再将车辆的位置坐标bbox传给pulc属性识别网络，最终由pulc属性识别网络确定车辆的车型以及车身颜色。车辆属性识别可以应用到交通安防领域，基于道路交通监控图像，识别各类车辆，综合车辆外观属性、车型等，形成完整的车辆画像，进行特定车辆的定位和追踪，为分析预警提供多维度参考依据。

本应用可以识别图像或视频中每个车辆，并返回该车辆的位置坐标、车型、车身颜色。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./vehicle_attr.elf<kmodel> <od_thresh> <nms_thresh> <input_mode> <attr_kmodel> <color_thresh> <type_thresh> <debug_mode>
For example:
 [for img] ./vehicle_attr.elf vehicle_attr_yolov5n.kmodel 0.1 0.45 car.jpg vehicle.kmodel 0.1 0.1 0
 [for isp] ./vehicle_attr.elf vehicle_attr_yolov5n.kmodel 0.1 0.45 None vehicle.kmodel 0.1 0.1 0
Options:
 1> kmodel          检测kmodel文件路径
 2> od_thresh       检测阈值
 3> nms_thresh      NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> attr_kmodel     属性识别kmodel文件路径
 6> color_thresh    颜色识别阈值
 7> type_thresh     车型识别阈值
 8> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/vehicle_attr/vehicle_attr_result.jpg" alt="车辆属性识别效果图" width="50%" height="50%" />