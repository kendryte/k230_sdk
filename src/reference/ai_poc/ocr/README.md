# 1.简介

OCR识别任务采用了CRNN网络结构，OCR检测任务采用了DBnet的网络结构。使用该应用，可检测到图像或视频中的文本位置以及相应的文字内容。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./ocr_rec.elf <kmodel_det> <threshold> <box_thresh> <input_mode> <kmodel_reco> <debug_mode>
Options:
  kmodel_det      ocr检测kmodel路径
  threshold       置信度阈值:影响检测框的大小，置信度阈值越小，检测框越大，也更容易检测到文字。
  box_thresh      Box阈值：影响检测框的多少，最后输出的检测框分数小于Box阈值的会被剔除，大于Box阈值的会保留，过大的Box阈值导致漏检，过小的Box阈值导致误检。
  input_mode      本地图片(图片路径)/ 摄像头(None)
  kmodel_reco     ocr识别kmodel路径
  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #单图推理示例：（ocr_img.sh）
./ocr_reco.elf ocr_det_int16.kmodel 0.25 0.4 333.jpg ocr_rec_int16.kmodel 0

 #视频流推理：（ocr_isp.sh）
./ocr_reco.elf ocr_det_int16.kmodel 0.25 0.4 None ocr_rec_int16.kmodel 0
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/ocr/ocr_result.jpg" alt="文字识别效果图" width="50%" height="50%"/>

