# 1.简介

人脸检测采用了retina-face网络结构，backbone选取0.25-mobilenet，人脸对齐网络基于3DDFA(3D Dense Face Alignment)实现。使用该应用，可得到图像或视频中的每个人脸的深度图（depth）或归一化投影坐标编码图（pncc）。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./face_alignment<kmodel_det> <obj_thres> <nms_thres> <kmodel_align> <kmodel_align_post> <input_mode> <output_mode> <debug_mode>
Options:
  kmodel_det               人脸检测kmodel路径
  obj_thres                人脸检测阈值
  nms_thres                人脸检测nms阈值
  kmodel_align             人脸对齐kmodel路径
  kmodel_align_post        人脸对齐后处理kmodel路径
  input_mode               本地图片(图片路径)/ 摄像头(None)
  output_mode              渲染方式（depth、pncc）
  debug_mode               是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（face_alignment_image.sh）
./face_alignment.elf face_detection_320.kmodel 0.6 0.2 face_alignment.kmodel face_alignment_post.kmodel 1024x768.jpg depth 1

 #视频流推理：（face_alignment_isp.sh）
./face_alignment.elf face_detection_320.kmodel 0.6 0.2 face_alignment.kmodel face_alignment_post.kmodel None depth 0
```
## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/face_alignment/face_depth.jpg" alt="人脸深度图" width="50%" height="50%"/>

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/face_alignment/face_pncc.jpg" alt="人脸pncc图" width="50%" height="50%"/>


