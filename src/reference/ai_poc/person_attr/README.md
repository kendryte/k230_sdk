# 1.简介

人体检测与属性识别应用可以检测图像中的所有人体，返回每个人体的位置坐标，并识别人体的属性信息，包含性别、年龄、佩戴眼镜、是否持物等，辅助定位追踪特定人员。

该应用的应用场景广泛。举例如下：

   1.安防监控

识别人体的性别年龄、衣着外观等特征，辅助定位追踪特定人员；监测预警各类危险、违规行为（如公共场所跑跳、抽烟），减少安全隐患

  2.智能零售

商场、门店等线下零售场景，识别入店及路过客群的属性信息，收集消费者画像，辅助精准营销、个性化推荐、门店选址、流行趋势分析等应用

  3.线下广告投放

楼宇、户外等广告屏智能化升级，采集人体信息，分析人群属性，定向投放广告物料，提升用户体验和商业效率。



# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./person_attr.elf <kmodel> <pd_thresh> <nms_thresh> <input_mode> <attr_kmodel> <pulc_thresh> <glasses_thresh> <hold_thresh> <debug_mode>
For example:
 [for img] ./person_attr.elf person_attr_yolov5n.kmodel 0.5 0.45 hrnet_demo.jpg person_pulc.kmodel 0.5 0.5 0.5 0
 [for isp] ./person_attr.elf person_attr_yolov5n.kmodel 0.5 0.45 None person_pulc.kmodel 0.5 0.5 0.5 0
Options:
 1> kmodel          行人检测kmodel文件路径
 2> pd_thresh       行人检测阈值
 3> nms_thresh      NMS阈值
 4> input_mode      本地图片(图片路径)/ 摄像头(None)
 5> attr_kmodel     属性识别kmodel文件路径
 6> pulc_thresh     属性识别阈值
 7> glasses_thresh  是否配戴眼镜阈值
 8> hold_thresh     是否持物阈值
 9> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

## 2.2 效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/person_attr/person_attr_result.jpg" alt="人体属性识别效果图" width="50%" height="50%" />

