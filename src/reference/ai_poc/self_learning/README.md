# 1.简介

自学习(self-taught learning) 是更为一般的、更强大的学习方式，它不要求未标注数据和已标注数据有同样的分布。自学习在无标注数据集的特征学习中应用更广。

假设有一个区分香蕉和葡萄的计算机视觉任务。为了获取大量的未标注数据，从互联网下载含有香蕉和葡萄的图像数据集，然后在这个图像数据集上训练稀疏自编码器。这个自编码器的输出是一个定长的向量，这个向量用于提取图像特征。这种情形就是“自学习”。

自学习包括两个部分，第一个是用无标签的样本集训练稀疏自编码器，第二个是用有标签的训练样本集训练一个分类器。

具体过程如下：

第一步，设置神经网络的结构；

第二步，利用无标签数据集训练稀疏自编码器；

第三步，利用稀疏自编码器对有标签的训练样本集和测试样本集提取特征；

第四步，利用第三步提取的训练集特征及其标签集**，**训练一个分类器。


# 2.应用使用说明

## 2.1 使用帮助

```
Usage: ./self_learning.elf <kmodel> <input_mode> <debug_mode> <topK> <mode>
For example:
 [for isp] ./self_learning.elf recognition.kmodel None 0 3 work
Options:
 1> kmodel          kmodel文件路径
 2> input_mode      摄像头(None)
 3> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 4> topK            返回topK结果
 5> mode            操作模式，[work] or [label]
```

## 2.2 操作步骤

### 2.2.1 创建类别层级目录

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/self_learning/1.jpg" alt="自学习效果图" width="50%" height="50%" />

首先，在小核界面执行 “./create_category” 命令，然后按照提示，依次输入类别，比如“apple”、“pear”、“banana”、“grape”、“orange”，自动创建类别层级目录。 可执行文件 create_category 是由项目工程 self_learning_small 生成的。

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/self_learning/2.jpg" alt="自学习效果图" width="50%" height="50%" />



### 2.2.2 标注【label】

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/self_learning/label.gif" alt="自学习效果图" width="50%" height="50%" />

在大核界面，执行 

```
./self_learning_label.sh
```

查看 self_learning_label.sh ：( cat  self_learning_label.sh 即可查看 )  

```
./self_learning.elf recognition.kmodel None 0 3 label
```

每张图片按照不同角度拍摄 4~6 张即可。

可执行文件 self_learning.elf  是由 self_learning 项目工程生成的。

需要注意的是，退出【label】模式，需要 重启板子。重启命令：reboot。

### 2.2.3 检测识别【work】

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/self_learning/work_output.gif" alt="自学习效果图" width="50%" height="50%" />

在大核界面，执行 

```
./self_learning_work.sh
```

查看 self_learning_work.sh ：( cat  self_learning_work.sh 即可查看 )  

```
./self_learning.elf recognition.kmodel None 0 3 work
```
