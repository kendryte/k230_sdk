# 1.简介

异常检测方法在训练时只需要正样本图像即可进行模型训练，训练得到的异常检测模型能够从输入图片中辨别出正常图像和异常图像。该方法通常会被应用在工业图像检测、医疗图像分析、安防监控等领域。

本示例工程提供的模型使用patchcore异常检测方法训练得到，能够从输入图片中辨别出玻璃瓶口是否存在异常。

# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./anomaly_det.elf <kmodel_det> <obj_thres> <input_mode> <debug_mode>
Options:
  kmodel_det      异常检测kmodel路径
  obj_thres       异常检测阈值
  input_mode      本地图片(图片路径)/ 摄像头(None) 
  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试

 #单图推理示例：（anomaly_det_image.sh）
./anomaly_det.elf anomaly_det.kmodel 3.7 000.png 0
```
**注意：**1）使用异常检测方法时需要将memory.bin文件拷贝到anomaly_det.elf同级目录

​			2）异常检测方法只支持单图推理模式，不支持视频流推理

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/anomaly_det/anomaly_detection_result.jpg" alt="异常检测效果图" width="50%" height="50%"/>

