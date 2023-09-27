# 1.简介

手掌检测采用了yolov5网络结构，backbone选取了1.0-mobilenetV2，手掌关键点检测采用了resnet50网络结构。使用该应用，可得到图像或视频中的每个手掌的21个骨骼关键点位置，并且我们通过拇指中指来实现隔空缩放图像。

# 2.应用使用说明

## 2.1 使用帮助

```
"Usage: " << space_resize.elf << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <debug_mode>"

各参数释义如下：
kmodel_det      手掌检测 kmodel路径
obj_thresh      手掌检测阈值
nms_thresh      手掌检测非极大值抑制阈值
kmodel_kp       手掌关键点检测 kmodel路径
debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #视频流推理：（space_resize_isp.sh）
./space_resize.elf hand_det.kmodel 0.15 0.4 handkp_det.kmodel 0

操作说明：
首先固定中指和拇指的初始距离，将手置于摄像头前，缩放中指和拇指的距离，即可范围区域进行缩放。
```
## 2.2 效果展示
<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/space_resize/space_resize.gif" alt="video.gif" width="50%" height="50%" />



