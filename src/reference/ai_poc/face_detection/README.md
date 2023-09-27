# 1.简介

人脸检测采用了retina-face网络结构，backbone选取0.25-mobilenet。使用该应用，可得到图像或视频中的每个人脸检测框以及每个人脸的左眼球/右眼球/鼻尖/左嘴角/右嘴角五个关键点位置。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./face_detection.elf <kmodel_det> <obj_thres> <nms_thres> <input_mode> <debug_mode>

各参数释义如下：
 kmodel_det ：人脸检测kmodel文件路径
 obj_thres ：人脸检测阈值
 nms_thres：人脸检测非极大值抑制的阈值
 input_mode：本地图片(图片路径)/ 摄像头(None)
 debug_mode：是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（face_detect_image.sh）
./face_detection.elf face_detection_320.kmodel 0.6 0.2 1024x624.jpg 1

 #视频流推理：（face_detect_isp.sh）
./face_detection.elf face_detection_320.kmodel 0.6 0.2 None 0
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/face_detection/face_detect_result.jpg" alt="人脸检测效果图" width="50%" height="50%"/>



