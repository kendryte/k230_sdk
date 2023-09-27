import time
from .Baidu_Text_transAPI import translate
import warnings
warnings.filterwarnings('ignore')

# input_language: 'zh', 'en', 'jp'
# output_language: 'zh', 'en', 'jp'
def tts(text, wav_save_file, output_language='zh'):
    """
    para:
    text:需要生成音频的文本
    wav_save_file:生成的单通道音频存放的文件夹位置
    output_language:生成的单通道音频的语言类型

    return:
    生成的单通道音频的存放路径
    双声道音频的名称
    """
    fanyi_tic = time.time()
    text = translate(text, output_language)['trans_result'][0]['dst']
    fanyi_toc = time.time()

    print(text)
    if output_language == 'zh':
        from .tts_chinese_and_english import tts_zh
        tts_tic = time.time()
        tts_zh(text, wav_save_file)
        tts_toc = time.time()
        print(f'翻译耗时为：{fanyi_toc - fanyi_tic}秒')
        print(f'声音生成耗时为：{tts_toc - tts_tic}秒')
        return f'{wav_save_file}/zh_mono.wav', 'zh_stereo.wav'
    elif output_language == 'en':
        from .tts_chinese_and_english import tts_en
        tts_tic = time.time()
        tts_en(text, wav_save_file)
        tts_toc = time.time()
        print(f'翻译耗时为：{fanyi_toc - fanyi_tic}秒')
        print(f'声音生成耗时为：{tts_toc - tts_tic}秒')
        return f'{wav_save_file}/en_mono.wav', 'en_stereo.wav'
    else:
        from .tts_japanese import tts_jp
        tts_tic = time.time()
        tts_jp(text, wav_save_file)
        tts_toc = time.time()
        print(f'翻译耗时为：{fanyi_toc - fanyi_tic}秒')
        print(f'声音生成耗时为：{tts_toc - tts_tic}秒')
        return f'{wav_save_file}/jp_mono.wav', 'jp_stereo.wav'

if __name__ == '__main__':
    text = "如同飞机不是飞的更高的鸟儿一样，人工智能也并不是更聪明的人。" \
           "在大数据之前，计算机并不擅长解决需要人类智能的问题。但是今天这些问题换个思路就可以解决了，其核心就是变智能问题为数据问题。" \
           "由此，全世界开始了新的一个人工智能的时代，人们开始意识到数据的重要性。随着技术的不断发展和计算能力的提升，人工智能正以前所未有的速度渗透到各个领域。"
    # input_language = 'zh'
    output_language = 'jp'
    tts(text, output_language)