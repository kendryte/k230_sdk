# 1.简介

交通信号灯检测是检验自动驾驶能力最重要的一环，真实的交通信号灯位置各异、朝向不同。这无疑增加了识别难度。交通信号灯属于小目标，在一张图像中所占的像素比例极小，所以模型提取的特征有限。交通信号灯的状态实时变化，这无疑又提升了检测难度。另外，在不同光照条件下，红灯和黄灯极难区分。

本应用采用了yolov5网络结构。使用该应用，可检测到图像或视频中的交通信号灯，且能很好地区分红灯和黄灯。

# 2.应用使用说明

## 2.1 使用帮助

```
 Usage: ./traffic_light_detect.elf <kmodel> <tld_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for img] ./traffic_light_detect.elf traffic.kmodel 0.5 0.45 traffic.jpg 0
 [for isp] ./traffic_light_detect.elf traffic.kmodel 0.5 0.45 None 0
Options:
 1> kmodel          交通信号灯检测kmodel文件路径
 2> tld_thresh      交通信号灯检测阈值
 3> nms_thresh      NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/traffic_light_detect/td_result_111_r.jpg" alt="交通信号灯检测效果图" width="50%" height="50%" />

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/traffic_light_detect/td_result_222_r.jpg" alt="交通信号灯检测效果图" width="50%" height="50%" />

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/traffic_light_detect/td_result_333_r.jpg" alt="交通信号灯检测效果图" width="50%" height="50%" />