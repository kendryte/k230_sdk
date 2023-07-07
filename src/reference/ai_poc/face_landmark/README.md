# 1.简介

人脸检测采用了retina-face网络结构，人脸密集关键点检测backbone选取0.5-mobilenet。使用该应用，可得到图像或视频中的每个人根据106关键点绘制的五官轮廓。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./face_landmark.elf<kmodel_det> <obj_thres> <nms_thres> <kmodel_fkp> <input_mode> <debug_mode>
Options:
  kmodel_det      人脸检测kmodel路径
  obj_thres       人脸检测阈值
  nms_thres       人脸检测nms阈值
  kmodel_fkp      密集人脸关键点kmodel路径
  input_mode      本地图片(图片路径)/ 摄像头(None)
  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（face_landmark_image.sh）
./face_landmark.elf face_detection_320.kmodel 0.6 0.2 face_landmark.kmodel 1024x1331.jpg 2

 #视频流推理：（face_landmark_isp.sh）
./face_landmark.elf face_detection_320.kmodel 0.6 0.2 face_landmark.kmodel None 0
```



