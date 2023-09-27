# 1.简介

多目标分割采用了yolov8网络结构。使用该应用，可得到图像或视频中的每个目标的分割掩码。

# 2.应用使用说明

## 2.1 使用帮助
开发板需求：需要用lp4(2G)、lp3(512M)的开发板

```
"Usage: " << seg.elf << "<kmodel_seg> <conf_thres> <nms_thres> <mask_thres> <input_mode> <debug_mode>"

各参数释义如下：
kmodel_seg     多目标分割 kmodel路径
conf_thres        多目标分割 分数阈值
nms_thres        多目标分割 非极大值抑制阈值
mask_thres      多目标分割 掩码阈值
input_mode     本地图片(图片路径)/ 摄像头(None) 
debug_mode   是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（segment_image_320.sh）（segment_image_640.sh）
./seg.elf yolov8n_seg_320.kmodel 0.1 0.5 0.5 bus.jpg 0
./seg.elf yolov8n_seg_640.kmodel 0.1 0.5 0.5 bus.jpg 0

 #视频流推理：（segment_isp_320.sh）（segment_isp_640.sh）
./seg.elf yolov8n_seg_320.kmodel 0.1 0.5 0.5 None 0
./seg.elf yolov8n_seg_640.kmodel 0.1 0.5 0.5 None 0


操作说明：
将摄像头置于需分割目标前即可在显示屏中观察到该目标的分割效果。
```
## 2.2 效果展示
<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/segment_yolov8n/person.jpg" alt="image.jpg" width="50%" height="50%" />



