# 1.简介

nanotracker 是单目标跟踪应用，也是多模型、双输入、双输出的实例。框定目标之后，本应用可以实现实时跟踪该目标。

需要注意的是，千万不要移动太快，否则小tracker会跟不上。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./nanotracker.elf<crop_kmodel> <src_kmodel> <head_kmodel> <crop_net_len> <src_net_len> <head_thresh>  <debug_mode>
For example:
 [for isp]  ./nanotracker.elf cropped_test127.kmodel nanotrack_backbone_sim.kmodel nanotracker_head_calib_k230.kmodel 127 255 0.1 0
Options:
 1> crop_kmodel    模板template kmodel文件路径
 2> src_kmodel  跟踪目标 kmodel文件路径
 3> head_kmodel  检测头 kmodel文件路径
 4> crop_net_len     模板template 模型输入尺寸
 5> src_net_len    跟踪目标 kmodel 模型输入尺寸
 6> head_thresh    检测头 检测阈值
 7> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

