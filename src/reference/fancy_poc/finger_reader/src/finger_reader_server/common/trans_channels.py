import numpy as np
import soundfile as sf

# 指定单通道 WAV 文件的路径
def trans_mono_to_stereo(mono_filepath, stereo_filepath):
    # 使用 soundfile 库加载单通道 WAV 文件
    wav_mono, sr = sf.read(mono_filepath)

    # 创建双通道的音频
    wav_stereo = np.column_stack((wav_mono, wav_mono))

    # 使用 soundfile 库保存双通道 WAV 文件
    sf.write(stereo_filepath, wav_stereo, sr, format='WAV', subtype='PCM_16')


    # print('Conversion complete.')