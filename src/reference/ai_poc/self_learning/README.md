# 1.简介

自学习(self-taught learning) 是更为一般的、更强大的学习方式，它不要求未标注数据和已标注数据有同样的分布。自学习在无标注数据集的特征学习中应用更广。

假设有一个区分香蕉和葡萄的计算机视觉任务。为了获取大量的未标注数据，从互联网下载含有香蕉和葡萄的图像数据集，然后在这个图像数据集上训练稀疏自编码器。这个自编码器的输出是一个定长的向量，这个向量用于提取图像特征。这种情形就是“自学习”。

此方法采用一个已经训练完成的模型来获取目标的特征向量，利用此向量来注册或者识别目标物体。


# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./self_learning.elf <kmodel> <crop_w> <crop_h> <thres> <topk> <debug_mode>
Options:
    1> kmodel         kmodel文件路径
    2> crop_w         剪切范围w
    3> crop_h         剪切范围h
    4> thres          判别阈值
    5> topk           识别范围
    6> debug_mode     是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
```

## 2.2 操作步骤

视频流推理：self_learning.sh  
./self_learning.elf recognition.kmodel 400 400 0.5 3 0

操作说明：  
1.启动程序  
2.将需要自学习的物体置于camera前，物体需要在显示器所画的框内  
3.键盘按 i 进入：增加新特征 or 删除已有特征 阶段  
&emsp;a.输入n 新建特征， 为特征取名字 format : {category}_{index}.bin  eg: apple_0.bin  
&emsp;b.输入d 删除特征， 删除对应特征的索引， 输入前会有提示：Already have : | 0 -> apple_0.bin | 1 _> banana_0.bin  
4.如果不需要删除已有特征或者增加新的特征，跳过第3步即可识别物体类别  

## 2.2 效果展示
<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/self_learning/self_learning.gif" alt="自学习展示.gif" width="50%" height="50%" />