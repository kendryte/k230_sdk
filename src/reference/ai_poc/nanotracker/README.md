# 1.简介

目标跟踪是指对图像序列中的运动目标进行检测、提取、识别跟踪，获得运动目标的运动参数，如位置、速度、加速度和运动轨迹等，从而进行下一步的处理与分析，实现对运动目标的行为理解，以完成更高一级的检测任务。对于单目标跟踪而言，在第一帧给出待跟踪的目标，在后续帧中，tracker能够自动找到目标并用bbox标出。关于单目标跟踪，有两种思路。第一种是把跟踪粗暴地当做一个配对问题，即把第一帧的目标当做模板，去匹配其他帧。基于这种思路，网络并不需要“理解”目标，只需当新的一帧图像来到时，拿着模板“连连看”找相同就可。每次两个输入，模板和新图片，然后通过网络在新图片上找和模板最相似的东西，所以这条思路的关键在于如何配得准。另一种思路是通过第一帧给出的目标去“理解”目标，在后续帧中，不需要再输入模板，即只有一个输入，网络可以根据自己理解的模板，在新图片中预测出目标，所以这条思路的关键在于如何让网路仅仅看一眼目标(第一帧)就能向目标检测那样，“理解”目标，这就涉及到单样本学习问题，也是检测和跟踪的gap。

本应用采用了第一种思路。本应用是多模型、双输入、双输出的实例。在框定目标之后，本应用可以实时跟踪该目标。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./nanotracker.elf<crop_kmodel> <src_kmodel> <head_kmodel> <crop_net_len> <src_net_len> <head_thresh>  <debug_mode>
For example:
 [for isp]  ./nanotracker.elf cropped_test127.kmodel nanotrack_backbone_sim.kmodel nanotracker_head_calib_k230.kmodel 127 255 0.1 0
Options:
 1> crop_kmodel    模板template kmodel文件路径
 2> src_kmodel     跟踪目标 kmodel文件路径
 3> head_kmodel    检测头 kmodel文件路径
 4> crop_net_len   模板template 模型输入尺寸
 5> src_net_len    跟踪目标 kmodel 模型输入尺寸
 6> head_thresh    检测头 检测阈值
 7> debug_mode     是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

## 2.2 操作步骤

### 2.2.1 框定目标

当屏幕显示绿色框时，请将目标移入框中。

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/nanotracker/output1.gif" alt="单目标跟踪操作指南" width="50%" height="50%" />

### 2.2.2 移动目标

当绿色框变为红色框时，即可移动目标。

需要注意的是，千万不要移动太快，否则小tracker可能会跟不上。

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/nanotracker/output2.gif" alt="单目标跟踪操作指南" width="50%" height="50%" />

如果框大小不合适，请修改源码文件  cv2_utils.h  第42行  MARGIN 系数

```
#define MARGIN  0.4  //   目标框缩放调整系数 
```

# 3.效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/nanotracker/nanotracker.gif" alt="单目标跟踪效果图" width="50%" height="50%" />

以上效果图是在EVB板子实现的。