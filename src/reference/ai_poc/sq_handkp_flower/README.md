# 1.简介

手掌检测采用了yolov5网络结构，backbone选取了1.0-mobilenetV2，手掌关键点检测采用了resnet50网络结构。花卉识别backbone选取了1.0-mobilenetV2。使用该应用，可得到图像或视频中的两个手掌的食指指尖包围区域内的花卉类别。可支持102种花卉的种类识别，具体种类可参考 [102 Category Flower Dataset](https://www.robots.ox.ac.uk/~vgg/data/flowers/102/) 。

# 2.应用使用说明

## 2.1 使用帮助

```
"Usage: " << sq_handkp_flower.elf << "<kmodel_det> <input_mode> <obj_thresh> <nms_thresh> <kmodel_kp> <flower_rec> <debug_mode>"

各参数释义如下：
kmodel_det      手掌检测 kmodel路径
input_mode      本地图片(图片路径)/ 摄像头(None) 
obj_thresh      手掌检测阈值
nms_thresh      手掌检测非极大值抑制阈值
kmodel_kp       手掌关键点检测 kmodel路径
flower_rec      花卉识别kmodel路径
debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（handkpflower_image.sh）
./sq_handkp_flower.elf hand_det.kmodel input_flower.jpg 0.15 0.4 handkp_det.kmodel flower_rec.kmodel 0

 #视频流推理：（handkpflower_isp.sh）
./sq_handkp_flower.elf hand_det.kmodel None 0.15 0.4 handkp_det.kmodel flower_rec.kmodel 0
```
## 2.2 效果展示
![demo示例](https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/sq_handkp_flower/handkp_flower.jpg){:height="50%" width="50%"}

**注意：** 当两只手同时稳定出现在摄像头下才会做花卉识别，正确使用方式请参考上图。本应用仅提供一个示例实现，如需更高精度或更优体验，可通过调整阈值或自行替换相应的模型文件。
