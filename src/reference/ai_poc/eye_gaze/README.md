# 1.简介

注视估计应用根据人脸预测人正在看哪里。该应用采用人脸检测和注视估计2个模型，对视频帧或图片，先进行人脸检测，然后对每个人脸进行注视估计，预测出注视向量，并以箭头的方式显示到屏幕上。注视估计可以应用到汽车安全领域，如利用注视向量查看的车辆内的相对位置，甚至是具体部件，以此来达到人机交互的目的。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./eye_gaze <kmodel_det> <obj_thres> <nms_thres> <kmodel_gaze> <input_mode> <debug_mode>
Options:
  kmodel_det               人脸检测kmodel路径
  obj_thres                人脸检测阈值
  nms_thres                人脸检测nms阈值
  kmodel_gaze              注视估计kmodel路径
  input_mode               本地图片(图片路径)/ 摄像头(None)
  debug_mode               是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（eye_gaze_image.sh）
./eye_gaze.elf face_detection_320.kmodel 0.6 0.2 eye_gaze.kmodel 1024x1111.jpg 2

 #视频流推理：（eye_gaze_isp.sh）
./eye_gaze.elf face_detection_320.kmodel 0.6 0.2 eye_gaze.kmodel None 0
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/eye_gaze/eye_gaze.gif" alt="注视估计效果图" width="50%" height="50%"/>
