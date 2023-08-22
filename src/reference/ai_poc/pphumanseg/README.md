# 1.简介

人像分割采用了pphumanseg网络结构。使用该应用，可在图像或视频中实现每个人与背景分开。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./pphumanseg.elf <kmodel> <input_mode> <debug_mode>
For example:
 [for img] ./pphumanseg.elf human_seg_2023mar.kmodel 1000.jpg 0
 [for isp] ./pphumanseg.elf human_seg_2023mar.kmodel None 0
Options:
 1> kmodel    kmodel文件路径
 2> input_mode      本地图片(图片路径)/ 摄像头(None)
 3> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

