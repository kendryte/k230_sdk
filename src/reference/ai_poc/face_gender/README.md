# 1.简介

人脸性别分类应用使用两个模型实现判断图像/视频中每个人的性别的功能，每个人物性别用M或F表示，其中M表示男性（Male），F表示女性（Female）。人脸性别分类可以应用到个性化推荐领域。

人脸性别分类应用的两个模型分别是人脸检测检测模型和人脸性别分类模型，前者使用retina-face网络结构，得到人脸检测框；后者选用EfficientNetB3为backbone进行分类，得到人物性别。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./face_gender.elf<kmodel_det> <obj_thres> <nms_thres> <kmodel_fg> <input_mode> <debug_mode>
Options:
  kmodel_det      人脸检测kmodel路径
  obj_thres       人脸检测阈值
  nms_thres       人脸检测nms阈值
  kmodel_fg       人脸性别kmodel路径
  input_mode      本地图片(图片路径)/ 摄像头(None)
  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（face_gender_image.sh）
./face_gender.elf face_detection_320.kmodel 0.6 0.2 face_gender.kmodel 1024x768.jpg 0

 #视频流推理：（face_gender_isp.sh）
./face_gender.elf face_detection_320.kmodel 0.6 0.2 face_gender.kmodel None 0
```
## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/face_gender/face_gender_result.png" alt="性别识别效果图" width="50%" height="50%"/>


