# 1.简介

手掌检测采用了yolov5网络结构，backbone选取了1.0-mobilenetV2，手掌关键点检测采用了resnet50网络结构。使用该应用，可得到图像或视频中的每个手掌的21个骨骼关键点位置。

# 2.应用使用说明

## 2.1 使用帮助

```
"Usage: " << sq_handkp_det.elf << "<kmodel_det> <input_mode> <obj_thresh> <nms_thresh> <kmodel_kp> <debug_mode>"

各参数释义如下：
kmodel_det      手掌检测 kmodel路径
input_mode      本地图片(图片路径)/ 摄像头(None) 
obj_thresh      手掌检测阈值
nms_thresh      手掌检测非极大值抑制阈值
kmodel_kp       手掌关键点检测 kmodel路径
debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（handkpdet_cpp_image.sh）
./sq_handkp_det.elf hand_det.kmodel input_hd.jpg 0.15 0.4 handkp_det.kmodel 0

 #视频流推理：（handkpdet_cpp_isp.sh）
./sq_handkp_det.elf hand_det.kmodel None 0.15 0.4 handkp_det.kmodel 0
```
**注意：** 本应用仅提供一个示例实现，如需更高精度或更优体验，可通过调整阈值或自行替换相应的模型文件。


