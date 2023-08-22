# 1.简介

人脸检测采用了retina-face网络结构，人脸口罩分类backbone选取mobilenet-v2。使用该应用，可得到图像或视频中的每个人是否佩戴口罩。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./face_mask.elf<kmodel_det> <obj_thres> <nms_thres> <kmodel_fm> <mask_thres> <input_mode> <debug_mode>
Options:
  kmodel_det      人脸检测kmodel路径
  obj_thres       人脸检测阈值
  nms_thres       人脸检测nms阈值
  kmodel_fm       人脸口罩kmodel路径
  mask_thres      人脸口罩阈值
  input_mode      本地图片(图片路径)/ 摄像头(None)
  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（face_mask_image.sh）
./face_mask.elf face_detection_320.kmodel 0.6 0.2 face_mask.kmodel 0.5 1024x768.jpg 2

 #视频流推理：（face_mask_isp.sh）
./face_mask.elf face_detection_320.kmodel 0.6 0.2 face_mask.kmodel 0.5 None 0
```



