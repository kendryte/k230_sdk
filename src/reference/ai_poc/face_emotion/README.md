# 1.简介

人脸检测采用了retina-face网络结构，人脸情感识别backbone选取mobilenet。使用该应用，可得到图像或视频中的每个人表情识别结果，分类包括Anger、Disgust、Fear、Happiness、Neutral、Sadness、Surprise。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./face_emotion.elf<kmodel_det> <obj_thres> <nms_thres> <kmodel_femo> <input_mode> <debug_mode>
Options:
  kmodel_det               人脸检测kmodel路径
  obj_thres                人脸检测阈值
  nms_thres                人脸检测nms阈值
  kmodel_femo              人脸情感识别kmodel路径
  input_mode               本地图片(图片路径)/ 摄像头(None)
  debug_mode               是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（face_emotion_image.sh）
./face_emotion.elf face_detection_320.kmodel 0.6 0.2 face_emotion.kmodel 1024x768.jpg 2

 #视频流推理：（face_emotion_isp.sh）
./face_emotion.elf face_detection_320.kmodel 0.6 0.2 face_emotion.kmodel None 0
```



