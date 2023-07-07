# 1.简介

人脸检测采用了retina-face网络结构，人脸眼镜分类backbone选取SqueezeNet-1.1。使用该应用，可得到图像或视频中的每个人是否佩戴眼镜。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./face_glasses.elf <kmodel_det> <obj_thres> <nms_thres> <kmodel_fg> <input_mode> <debug_mode>
Options:
  kmodel_det      人脸检测kmodel路径
  obj_thres       人脸检测阈值
  nms_thres       人脸检测nms阈值
  kmodel_fg       人脸眼镜kmodel路径
  input_mode      本地图片(图片路径)/ 摄像头(None)
  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（face_glasses_image.sh）
./face_glasses.elf face_detection_320.kmodel 0.4 0.2 face_glasses.kmodel 1024x768.jpg 2

 #视频流推理：（face_glasses_isp.sh）
./face_glasses.elf face_detection_320.kmodel 0.4 0.2 face_glasses.kmodel None 0
```



