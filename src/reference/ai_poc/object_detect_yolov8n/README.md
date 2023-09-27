# 1.简介

多目标检测采用了yolov8网络结构。使用该应用，可得到图像或视频中的每个目标的检测框。

# 2.应用使用说明

## 2.1 使用帮助

```
"Usage: " << ob_det.elf << "<kmodel_det> <score_thres> <nms_thres> <input_mode> <debug_mode>"

各参数释义如下：
kmodel_det     多目标检测 kmodel路径
score_thres      多目标检测 分数阈值
nms_thres        多目标检测 非极大值抑制阈值
input_mode     本地图片(图片路径)/ 摄像头(None) 
debug_mode   是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（ob_det_image_320.sh）（ob_det_image_640.sh）
./ob_det.elf yolov8n_320.kmodel 0.15 0.2 bus.jpg 0
./ob_det.elf yolov8n_640.kmodel 0.15 0.2 bus.jpg 0

 #视频流推理：（ob_detect_isp_320.sh）（ob_detect_isp_640.sh）
./ob_det.elf yolov8n_320.kmodel 0.15 0.2 None 0
./ob_det.elf yolov8n_640.kmodel 0.15 0.2 None 0

操作说明：
将摄像头置于检测目标前即可在显示屏中观察到该目标的检测效果。
```
## 2.2 效果展示
<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/object_detect_yolov8n/person.jpg" alt="image.jpg" width="45%" height="45%" /> <img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/object_detect_yolov8n/bird.jpg" alt="image0.jpg" width="45%" height="45%" />


