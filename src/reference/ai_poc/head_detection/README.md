# 1.简介

人头检测采用了yolov8网络结构。使用该应用，可得到图像或视频中的每个人头的坐标信息和总的人头个数，可以用于人头计数、人头跟踪等应用。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./head_detection.elf<kmodel_det> <score_thres> <nms_thres> <input_mode> <debug_mode>
各参数释义如下：
  kmodel_det      人头检测kmodel路径
  score_thres     人头检测阈值
  nms_thres       人头检测非极大值抑制阈值
  input_mode      本地图片(图片路径)/ 摄像头(None)
  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（head_detect_image.sh）
./head_detection.elf head_detection.kmodel 0.4 0.3 640x340.jpg 0

 #视频流推理：（head_detect_isp.sh）
./head_detection.elf head_detection.kmodel 0.4 0.3 None 0
```
## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/head_detection/head_detection_result.jpg" alt="人头计数效果图" width="50%" height="50%"/>

