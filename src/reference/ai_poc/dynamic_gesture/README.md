# 1.简介

手掌检测采用了yolov5网络结构，backbone选取了1.0-mobilenetV2，手掌关键点检测采用了resnet50网络结构,动态手势识别采用了tsm结构，backbone选取了mobilenetV2。使用该应用，可得到视频中的动态手势。若用在手机交互过程中，可对识别到的不同的手势定义不同的动作，如向上挥动手掌时手机页面向下滑动，捏合手掌时截屏等等。

# 2.应用使用说明

## 2.1 使用帮助

```
"Usage: " << dynamic_gesture.elf << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <kmodel_gesture> <debug_mode>"

各参数释义如下：
kmodel_det      手掌检测 kmodel路径
obj_thresh      手掌检测阈值
nms_thresh      手掌检测非极大值抑制阈值
kmodel_kp       手掌关键点检测 kmodel路径
kmodel_gesture  动态手势识别kmodel路径
debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #视频流推理：（gesture.sh）
./dynamic_gesture.elf hand_det.kmodel 0.4 0.4  handkp_det.kmodel gesture.kmodel 0
```

## 2.2 效果展示
![demo示例](https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/dynamic_gesture/dynamic_gesture.gif){:height="50%" width="50%"}

支持如上5个动态手势，分别是向下挥手、向上挥手、向左挥手、向右挥手、手掌和大拇指捏合。

**注意：** 在使用过程中，应先放置手掌在屏幕可见区域内，当屏幕左上角出现类似![demo示例](https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/dynamic_gesture/flag.jpg)标志时再开始挥动或者捏合手势。手部不宜离屏幕太近，以确保整个手部动作过程在摄像头可见区域内。本应用仅提供一个示例实现，如需更高精度或更优体验，可通过调整阈值或自行替换相应的模型文件。


