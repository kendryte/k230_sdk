# 1.简介

眼镜分类应用使用两个模型实现判断图像/视频每个人是否佩戴眼镜。在不可佩戴眼镜的应用场景中，若发现有人佩戴了眼镜，可进行相关提醒。

眼镜分类应用的两个模型分别是人脸检测检测模型和人脸眼镜分类模型，人脸检测检测模型使用retina-face网络结构，用于检测人脸框；人脸眼镜分类模型选用SqueezeNet-1.1为backbone，用于对每个人脸框判断眼镜佩戴情况。

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
./face_glasses.elf face_detection_320.kmodel 0.6 0.2 face_glasses.kmodel 1024x768.jpg 2

 #视频流推理：（face_glasses_isp.sh）
./face_glasses.elf face_detection_320.kmodel 0.6 0.2 face_glasses.kmodel None 0
```
## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/face_glasses/face_glasses_result.png" alt="眼镜效果图" width="50%" height="50%"/>


