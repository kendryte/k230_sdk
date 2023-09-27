# 1.简介

人脸姿态估计使用两个模型实现对图像/视频中每个人的脸部朝向的角度进行估计的功能。人脸朝向用一般用欧拉角（roll/yaw/pitch）表示，其中roll代表了人脸左右摇头的程度；yaw代表了人脸左右旋转的程度；pitch代表了人脸低头抬头的程度。该应用通过通过构建投影矩阵来可视化人脸朝向的变化，并且在图片推理过程中会将欧拉角可视化到图片推理结果中。

人脸姿态估计可用于许多业务场景，比如在人脸识别系统中，人脸姿态估计可以辅助进行输入样本的筛选；在疲劳驾驶产品可以用于评估驾驶员是否存在左顾右盼行为。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./face_pose.elf<kmodel_det> <obj_thres> <nms_thres> <kmodel_fp> <input_mode> <debug_mode>
Options:
  kmodel_det      人脸检测kmodel路径
  obj_thres       人脸检测阈值
  nms_thres       人脸检测nms阈值
  kmodel_fp       人脸姿态估计kmodel路径
  input_mode      本地图片(图片路径)/ 摄像头(None)
  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（face_landmark_image.sh）
./face_pose.elf face_detection_320.kmodel 0.6 0.2 face_pose.kmodel 1024x768.jpg 2

 #视频流推理：（face_landmark_isp.sh）
./face_pose.elf face_detection_320.kmodel 0.6 0.2 face_pose.kmodel None 0
```
## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/face_pose/face_pose_result.gif" alt="人脸解析效果图" width="50%" height="50%"/>



