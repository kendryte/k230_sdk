# 1.简介

人脸解析（人脸分割）应用使用两个模型实现对图像/视频中每个人脸的分割功能，人脸分割包含对人脸眼睛、鼻子、嘴巴等部位按照像素进行区分，不同的区域用不同的颜色表示。

人脸解析可以应用到虚拟化妆和美颜场景，通过将人脸从背景中分割出来，可以在人脸上实时应用虚拟化妆和美颜效果，提供更好的用户体验。也可以用于视频编辑和特效场景，在视频编辑中，人脸分割技术可以帮助实现对人脸的特效处理，比如在人脸上叠加特定的效果、文字或动态图像。

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
## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/face_parse/face_parse_result.png" alt="人脸解析效果图" width="50%" height="50%"/>


