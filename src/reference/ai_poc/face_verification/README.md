# 1.简介

人脸验证是一种基于人脸生物特征的身份验证技术，旨在确认个体是否是其所声称的身份。该技术通过分析和比对用户的脸部特征来验证其身份，通常是在人脸验证系统通过对比两张图片，确定两张图像中的人脸是否属于同一个人。

人脸验证在安全领域、用户身份验证方面有着广泛的应用，为各种应用场景提供了便捷、高效且安全的身份验证手段。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./face_verification.elf<kmodel_det> <det_thres> <nms_thres> <kmodel_recg> <recg_thres> <img_pth_A> <img_pth_B> <debug_mode>
Options:
  kmodel_det               人脸检测kmodel路径
  det_thres                人脸检测阈值
  nms_thres                人脸检测nms阈值
  kmodel_recg              人脸识别kmodel路径
  recg_thres               人脸识别阈值
  img_pth_A                本地图片A
  img_pth_B                本地图片B
  debug_mode               是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #推理示例：（face_verification_image.sh）
./face_verification.elf face_detection_320.kmodel 0.6 0.2 face_recognition.kmodel 75 identification_card.png person.png 1
```
## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/face_verification/face_verification_result.png" alt="人脸验证效果图" width="50%" height="50%"/>