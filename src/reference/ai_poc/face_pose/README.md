# 1.简介

人脸姿态估计，可得到图像或视频中的每个人脸的roll/yaw/pitch。roll代表了人头歪的程度；yaw代表了人头左右旋转的程度；pitch代表了人头低头抬头的程度。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./face_pose.elf<kmodel_det> <obj_thres> <nms_thres> <kmodel_fp> <input_mode> <debug_mode>
Options:
  kmodel_det      人脸检测kmodel路径
  obj_thres       人脸检测阈值
  nms_thres       人脸检测nms阈值
  kmodel_fp       人脸姿态估计kmodel路径
  input_mode      本地图片(图片路径)/ 摄像头(None)
  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（face_landmark_image.sh）
./face_pose.elf face_detection_320.kmodel 0.6 0.2 face_pose.kmodel 1024x768.jpg 2

 #视频流推理：（face_landmark_isp.sh）
./face_pose.elf face_detection_320.kmodel 0.6 0.2 face_pose.kmodel None 0
```



