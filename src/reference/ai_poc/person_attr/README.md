# 1.简介

人体属性识别采用了yolov5网络结构、pulc属性识别网络。使用该应用，可检测到图像或视频中的每个人的人体属性。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./person_attr.elf<kmodel> <pd_thresh> <nms_thresh> <input_mode> <attr_kmodel> <pulc_thresh> <glasses_thresh> <hold_thresh> <debug_mode>
For example:
 [for img] ./person_attr.elf person_attr_yolov5n.kmodel 0.5 0.45 hrnet_demo.jpg person_pulc.kmodel 0.5 0.5 0.5 0
 [for isp] ./person_attr.elf person_attr_yolov5n.kmodel 0.5 0.45 None person_pulc.kmodel 0.5 0.5 0.5 0
Options:
 1> kmodel    行人检测kmodel文件路径
 2> pd_thresh  行人检测阈值
 3> nms_thresh  NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> attr_kmodel 属性识别kmodel文件路径
 6> pulc_thresh 属性识别阈值
 7> glasses_thresh 是否配戴眼镜阈值
 8> hold_thresh 是否持物阈值
 9> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

【注意】

限于图片尺寸，只展示性别、年龄段、朝向三个属性。

