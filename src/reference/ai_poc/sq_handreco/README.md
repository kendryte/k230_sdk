# 1.简介

手掌检测采用了yolov5网络结构，backbone选取了1.0-mobilenetV2，手势识别backbone选取了1.0-mobilenetV2。使用该应用，可得到图像或视频中的每个手势的类别。可识别的类别如下以及其他手势：
![Alt text](https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/sq_handreco/img.png "demo示例")

# 2.应用使用说明

## 2.1 使用帮助

```
"Usage: " << sq_handreco.elf << "<kmodel_det> <input_mode> <obj_thresh> <nms_thresh> <kmodel_reco> <debug_mode>"

各参数释义如下：
kmodel_det      手掌检测 kmodel路径
input_mode      本地图片(图片路径)/ 摄像头(None) 
obj_thresh      手掌检测阈值
nms_thresh      手掌检测非极大值抑制阈值
kmodel_reco     手势识别 kmodel路径
debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（handreco_cpp_image.sh）
./sq_handreco.elf hand_det.kmodel input_hd.jpg 0.15 0.4 hand_reco.kmodel 0

 #视频流推理：（handreco_cpp_isp.sh）
./sq_handreco.elf hand_det.kmodel None 0.15 0.4 hand_reco.kmodel 0
```
**注意：** 本应用仅提供一个示例实现，如需更高精度或更优体验，可通过调整阈值或自行替换相应的模型文件。

