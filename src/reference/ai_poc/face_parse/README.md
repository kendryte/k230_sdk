# 1.简介

人脸检测采用了retina-face网络结构，人脸解析采用分割模型。使用该应用，可得到图像或视频中每个人脸眼睛、鼻子、嘴巴等部位进行分割。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./face_parse.elf<kmodel_det> <obj_thres> <nms_thres> <kmodel_fpa> <input_mode> <debug_mode>
Options:
  kmodel_det      人脸检测kmodel路径
  obj_thres       人脸检测阈值
  nms_thres       人脸检测nms阈值
  kmodel_fpa      人脸解析kmodel路径
  input_mode      本地图片(图片路径)/ 摄像头(None)
  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（face_parse_image.sh）
./face_parse.elf face_detection_320.kmodel 0.4 0.2 face_parse.kmodel 1024x768.jpg 0

 #视频流推理：（face_parse_isp.sh）
./face_parse.elf face_detection_320.kmodel 0.4 0.2 face_parse.kmodel None 0
```



