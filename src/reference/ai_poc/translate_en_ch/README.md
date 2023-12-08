# 1.简介

机器翻译模型应用了transformer结构。使用该应用，可以实现简单的英翻中翻译任务。

# 2.应用使用说明

## 2.1 使用帮助
开发板需求：当前模型无特殊要求；若是需要更大的模型，则需要用lp4(2G)开发板

```
"Usage: " << name << "<kmodel_encoder> <kmodel_decoder> <src_model_file> <tag_model_file> <debug_mode>"
各参数释义如下：
kmodel_encoder      编码器kmodel路径
kmodel_decoder      解码器kmodel路径
src_model_file      原语言分词器模型路径
tag_model_file      翻译后语言分词器模型路径
debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #推理示例：（translate_en_ch.sh）
translate_en_ch.elf translate_encoder.kmodel translate_decoder.kmodel trans_src.model trans_tag.model 0
```
## 2.2 效果展示
<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/translate_en_ch/translate_result.jpg" alt="translate_result.jpg" width="50%" height="50%" />




