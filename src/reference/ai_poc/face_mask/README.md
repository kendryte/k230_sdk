# 1.简介

口罩分类应用使用两个模型实现判断图像/视频每个人是否佩戴口罩。在需要佩戴口罩的应用场景中，若发现有人没有佩戴口罩，可进行相关提醒。

口罩分类应用的两个模型分别是人脸检测检测模型和人脸口罩分类模型，人脸检测检测模型使用retina-face网络结构，用于检测人脸框；人脸口罩分类模型使用mobilenet-v2为backbone，用于对每个人脸框判断口罩佩戴情况。

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

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/face_mask/face_mask_result.png" alt="口罩分类效果图" width="50%" height="50%"/>
