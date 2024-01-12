# 1.简介

行人测距是先通过行人检测检测行人，再通过检测框在图像中的大小去估算目标距离。其中行人检测采用了yolov5n的网络结构。使用该应用，可得到图像或视频中的每个行人的检测框以及估算的距离。该技术可应用在车辆辅助驾驶系统、智能交通等领域。

# 2.应用使用说明

## 2.1 使用帮助

```
"Usage: " << person_distance.elf << "<kmodel_det> <obj_thresh> <nms_thresh> <input_mode> <person_height> <debug_mode>"

各参数释义如下：
kmodel_det      行人检测 kmodel路径
obj_thresh      行人检测阈值
nms_thresh      行人检测非极大值抑制阈值
input_mode      本地图片(图片路径)/ 摄像头(None) 
person_height   行人身高，单位cm
debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试

 #单图推理：（person_distance_image.sh）
./person_distance.elf person_detect.kmodel 0.5 0.45 input_pd.jpg 170 0

 #视频流推理：（person_distance_isp.sh）
./person_distance.elf person_detect.kmodel 0.5 0.45 None 170 0
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/person_distance/pd_result.jpg" alt="行人测距效果图" width="50%" height="50%" />