# 1.简介

dms系统是以手掌检测和人脸检测为基础，再通过逻辑判断实现对行驶车辆司机的违规行为（抽烟、打电话、喝水）进行提醒。

人脸检测采用了retina-face网络结构，backbone选取0.25-mobilenet。使用该应用，可得到图像或视频中的每个人脸检测框以及每个人脸的左眼球/右眼球/鼻尖/左嘴角/右嘴角五个关键点位置。

手掌检测采用了yolov5网络结构，backbone选取了1.0-mobilenetV2。使用该应用，可得到图像或视频中的每个手掌的检测框。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./dms.elf <face_kmodel_det> <face_obj_thres> <face_nms_thres> <hand_kmodel_det> <hand_obj_thresh> <hand_nms_thresh> <init_area> <init_len> <warning_amount> <debug_mode>

各参数释义如下：
   face_kmodel_det      人脸检测kmodel路径
   face_obj_thres       人脸检测kmodel阈值
   face_nms_thres       人脸检测kmodel nms阈值
   hand_kmodel_det      手掌检测kmodel路径
   hand_obj_thresh      手掌检测阈值
   hand_nms_thresh      手掌检测非极大值抑制阈值
   init_area            定义基准人脸面积
   init_len             定义基准距离长度
   warning_amount       达到需要提醒注意的帧数
   debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #视频流推理：（dms_isp.sh）
./dms.elf face_detection_320.kmodel 0.6 0.2 hand_det.kmodel 0.15 0.2 60000 300 20 0
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/dms_system/dms.gif" alt="dms效果展示" width="50%" height="50%"/>



