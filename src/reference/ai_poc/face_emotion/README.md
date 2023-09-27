# 1.简介

面部表情识别使用两个模型实现图像/视频中每个人的表情识别的功能，可识别的表情类别包括Neutral、Happiness、Sadness、Anger、Disgust、Fear、Surprise。面部表情识别帮助我们更精准地了解人们的情绪和心理状态，该技术可以应用于人机交互、安全监控、医疗保健等领域。

面部表情识别应用的两个模型分别是人脸检测检测模型和面部表情识别模型，前者使用retina-face网络结构，得到人脸检测框；后者选用mobilenet为backbone进行分类，得到人物表情。

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
## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/face_emotion/face_emotion_result.png" alt="情感识别效果图" width="50%" height="50%"/>



