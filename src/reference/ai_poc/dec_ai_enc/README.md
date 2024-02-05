# 1.简介

dec_ai_enc(Decoder-AI-Encoder，解码器-编码器)，本项目使用嘉楠科技K230开发板实现了基于H265（HEVC）视频压缩标准的视频解码编码功能；项目包含enc、dec、dec_enc三个子项目。enc实现使用开发板自带sensor采集视频并压缩成码流文件filename.h265；dec_enc实现了将码流文件filename.h265文件解码为NV12(YUV420)格式，并实现RGB格式转换送入kmodel完成AI计算，然后将AI计算结果绘制在图像上，将多帧图像保存成码流文件new_filename.h265；dec实现了使用外接显示器解码new_filename.h265文件展示AI计算结果的功能。

项目实现功能结构如下：

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/dec_ai_enc/dec_enc_frame.png" alt="编解码流程" width="50%" height="50%"/>

本项目基于人脸检测功能作为示例，功能核心部分为dec_enc子项目，该部分可独立应用。

# 2.应用使用说明

## 2.1快速启动

使用串口通信工具连接大核，进入sharefs新建目录dec_enc，将dec.elf、dec_enc.elf、enc.elf、dec.sh、dec_enc.sh、enc.sh、face_detection_hwc.kmodel拷贝到dec_enc目录下，进入该目录，执行下述命令：

```shell
# ./enc.elf 0 -sensor 24 -o test.h265 采集h265码流
./enc.sh
# ./dec_enc.elf -i test.h265 -o test_new.h265 读入采集的码流文件，解码后送AI计算，将得到的结果绘制到每一帧图像上并编码成新的h265文件
./dec_enc.sh
# ./dec.elf -i test_new.h265 外接显示器展示AI计算结果
./dec.sh
```

***注意***：

1、人脸检测模型设定输入格式为RGB-HWC，请注意模型输入格式。

2、编解码器请注意16像素对齐，因此在采集码流文件时设置宽高分别为（1920，1088）。

3、K230视频编解码文档请参考：[k230_docs/zh/01_software/board/mpp/K230_视频编解码_API参考.md at main · kendryte/k230_docs (github.com)](https://github.com/kendryte/k230_docs/blob/main/zh/01_software/board/mpp/K230_视频编解码_API参考.md#324-k_vdec_chn_status) 。

## 2.2 效果展示

![效果展示](https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/dec_ai_enc/result.gif){:height="50%" width="50%"}
