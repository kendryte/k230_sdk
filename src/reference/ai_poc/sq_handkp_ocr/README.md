# 1.简介

手掌检测采用了yolov5网络结构，backbone选取了1.0-mobilenetV2，手掌关键点检测采用了resnet50网络结构。文字检测采用了retinanet网络结构，文字识别采用了以MobileNetV3为backbone的RLnet网络结构。使用该应用，可得到图像或视频中的每个手掌的食指左上区域范围内识别到的文字。

# 2.应用使用说明

## 2.1 使用帮助

```
"Usage: " << sq_handkp_ocr.elf << "<kmodel_det> <input_mode> <obj_thresh> <nms_thresh> <kmodel_kp> <kmodel_ocrdet> <threshold> <box_thresh> <kmodel_reco> <debug_mode>"

各参数释义如下：
kmodel_det      手掌检测 kmodel路径
input_mode      本地图片(图片路径)/ 摄像头(None) 
obj_thresh      手掌检测阈值
nms_thresh      手掌检测非极大值抑制阈值
kmodel_kp       手掌关键点检测 kmodel路径
kmodel_ocrdet   ocr检测kmodel路径\n"
threshold       ocr检测 threshold\n"
box_thresh      ocr检测 box_thresh\n"
kmodel_reco     ocr识别kmodel路径 \n"
debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（handkpocr_cpp_image.sh）
./sq_handkp_ocr.elf hand_det.kmodel input_ocr.jpg 0.15 0.4 handkp_det.kmodel ocr_det.kmodel 0.15 0.4 ocr_rec.kmodel 0

 #视频流推理：（handkpocr_cpp_isp.sh）
./sq_handkp_ocr.elf hand_det.kmodel None 0.15 0.4 handkp_det.kmodel ocr_det.kmodel 0.15 0.4 ocr_rec.kmodel 0
```



