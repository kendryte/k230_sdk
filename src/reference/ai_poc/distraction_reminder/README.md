# 1.简介

分神提醒主要采用了人脸姿态估计作为基础，通过逻辑判断实现对司机注意力不集中于前方的提醒。
人脸姿态估计使用两个模型实现对图像/视频中每个人的脸部朝向的角度进行估计的功能。人脸朝向用一般用欧拉角（roll/yaw/pitch）表示，其中roll代表了人脸左右摇头的程度；yaw代表了人脸左右旋转的程度；pitch代表了人脸低头抬头的程度。该应用通过通过构建投影矩阵来可视化人脸朝向的变化，并且在图片推理过程中会将欧拉角可视化到图片推理结果中。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./distraction_reminder.elf<kmodel_det> <obj_thres> <nms_thres> <kmodel_fp> <warning_amount> <warning_angle_roll> <warning_angle_yaw> <warning_angle_pitch> <debug_mode>
Options:
   kmodel_det           人脸检测kmodel路径
   obj_thres            人脸检测阈值
   nms_thres            人脸检测nms阈值
   kmodel_fp            人脸姿态估计kmodel路径
   warning_amount       达到需要提醒注意的帧数
   warning_angle_roll   达到需要提醒注意的滚转角偏离角度
   warning_angle_yaw    达到需要提醒注意的偏航角偏离角度
   warning_angle_pitch  达到需要提醒注意的俯仰角偏离角度
   debug_mode           是否需要调试，0、1、2分别表示不调试、简单调试、详细调试

 #视频流推理：（distraction_reminder_isp.sh）
./distraction_reminder.elf face_detection_320.kmodel 0.6 0.2 face_pose.kmodel 20 20 20 20 0
```
## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/distraction_reminder/distraction_reminder.gif" alt="分神提醒效果" width="50%" height="50%"/>



