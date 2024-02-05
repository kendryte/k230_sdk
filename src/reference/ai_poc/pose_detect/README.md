# 1.简介

人体关键点检测是一项涉及识别图像或视频中人体对象特定点（通常称为关键点）位置的任务。关键点可以表示人体对象的各个部分，例如关节或其他独特特征。关键点的位置通常表示为一组 2D [x, y] 或 3D [x, y,visible] 坐标。

人体关键点检测模型的输出是一组代表图像或视频中人体对象上的关键点，以及每个点的置信度得分。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./pose_detect.elf<kmodel> <obj_thresh> <nms_thresh> <input_mode> <debug_mode>
For example:
 [for img] ./pose_detect.elf yolov8n-pose.kmodel 0.3 0.45 bus.jpg 0
 [for isp] ./pose_detect.elf yolov8n-pose.kmodel 0.3 0.45 None 0
Options:
 1> kmodel          pose检测kmodel文件路径
 2> obj_thresh      pose检测阈值
 3> nms_thresh      NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/pose_detect/pose_result.jpg" alt="人体关键点检测效果图" width="50%" height="50%" />