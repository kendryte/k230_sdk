# 1.简介

人像分割指对图片或视频中的人体轮廓范围进行识别，将其与背景进行分离，返回分割后的二值图、灰度图、前景人像图等，实现背景图像的替换与合成。 可应用于人像抠图、照片合成、人像特效、背景特效等场景，大大提升图片和视频工具效率。

本应用采用了pphumanseg模型。使用该应用，可在图像或视频中实现每个人与背景分开。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./pphumanseg.elf <kmodel> <input_mode> <debug_mode>
For example:
 [for img] ./pphumanseg.elf human_seg_2023mar.kmodel 1000.jpg 0
 [for isp] ./pphumanseg.elf human_seg_2023mar.kmodel None 0
Options:
 1> kmodel          kmodel文件路径
 2> input_mode      本地图片(图片路径)/ 摄像头(None)
 3> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

