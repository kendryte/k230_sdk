# 1.简介

中文文字转语音（text to chinese speech, tts_zh）使用三个模型实现。用户默认输入三次文字，生成文字对应的wav文件。

tts_zh工程将FastSpeech2模型拆分成两个模型，Encoder+Variance Adaptor为fastspeech1，Decoder为fastspeech2，声码器选择hifigan。持续时间特征在fastspeech1之后添加。

# 2.应用使用说明

## 2.1使用帮助

```shell
Usage: ./tts_en.elf <fastspeech1_kmodel> <fastspeech2_kmodel> <hifigan_kmodel><debug_mode>
Options:
	fastspeech1_kmodel      第一阶段模型fastspeech1 kmodel路径
	fastspeech2_kmodel      第二阶段模型fastspeech2 kmodel路径			
	hifigan_kmodel          声码器模型hifigan kmodel路径
	debug_mode              是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
# 推理(tts_zh.sh)
./tts_zh.elf zh_fastspeech_1.kmodel zh_fastspeech_2.kmodel hifigan.kmodel 0
# 连续输入三次文字后，播放生成的音频
./wav_play.elf zh_0.wav
./wav_play.elf zh_1.wav
./wav_play.elf zh_2.wav
```

## 2.2效果展示

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/tts_zh/tts_zh.png" alt="tts_zh应用图示" width="50%" height="50%"/>