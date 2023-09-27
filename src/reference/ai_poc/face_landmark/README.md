# 1.简介

人脸密集关键点检测应用使用两个模型实现检测图像/视频中每张人脸的106关键点，并根据106关键点绘制人脸、五官等轮廓，不同轮廓使用不用的颜色表示。人脸密集关键点检测可以应用到人脸美颜、人脸贴纸、人脸驱动等领域中。

人脸密集关键点检测应用的两个模型分别是人脸检测检测模型和人脸密集关键点检测模型，前者使用retina-face网络结构，用于检测人脸框；后者选用0.5-mobilenet为backbone，用于对每张人脸检测106个关键点，106关键点包括人脸的脸颊、嘴巴、眼睛、鼻子和眉毛区域。

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
./face_landmark.elf face_detection.kmodel 0.6 0.2 face_landmark.kmodel 1024x1331.jpg 2

 #视频流推理：（face_landmark_isp.sh）
./face_landmark.elf face_detection_320.kmodel 0.6 0.2 face_landmark.kmodel None 0
```
## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/face_landmark/face_landmark_result.png" alt="密集关键点效果图" width="50%" height="50%"/>


