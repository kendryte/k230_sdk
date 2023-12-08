# 1.简介

手掌检测采用了yolov5网络结构，backbone选取了1.0-mobilenetV2，手掌关键点检测采用了resnet50网络结构。使用该应用，可以使用屏幕上的虚拟键盘输出字符。

# 2.应用使用说明

## 2.1 使用帮助

```
"Usage: " << virtual_keyboard.elf << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <debug_mode>"

各参数释义如下：
kmodel_det      手掌检测 kmodel路径
obj_thresh      手掌检测阈值
nms_thresh      手掌检测非极大值抑制阈值
kmodel_kp       手掌关键点检测 kmodel路径
debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试

 #视频流推理：（virtual_keyboard.sh）
./virtual_keyboard.elf hand_det.kmodel 0.25 0.4 handkp_det.kmodel 0
```

## 2.2 效果展示
![demo示例](https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/virtual_keyboard/virtual_keyboard.gif){:height="50%" width="50%"}

其中 ```Space```空格键，```clr```清空键，```<--```
退格键。

**注意：** 手部不宜离屏幕太近，确保手掌在摄像头可见区域内。大拇指指尖选择字符，食指指尖触碰大拇指指尖输出当前选择的字符。推荐使用方式请参考上图。本应用仅提供一个示例实现，如需更高精度或更优体验，可通过调整阈值或自行替换相应的模型文件。


