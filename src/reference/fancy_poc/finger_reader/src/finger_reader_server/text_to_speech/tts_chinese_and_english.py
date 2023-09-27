from paddlespeech.cli.tts.infer import TTSExecutor
import time
import warnings
warnings.filterwarnings('ignore')

def tts_zh(text, wav_save_file):
    tts = TTSExecutor()
    AM = 'speedyspeech_csmsc'
    VOC = 'pwgan_csmsc'
    tts(text=text,
        am=f'{AM}',
        voc=f'{VOC}',
        lang='zh',
        spk_id=0,
        output=f"{wav_save_file}/zh_mono.wav")

def tts_en(text, wav_save_file):
    tts = TTSExecutor()
    AM = 'fastspeech2_ljspeech'
    VOC = 'pwgan_ljspeech'
    tts(text=text,
        am=f'{AM}',
        voc=f'{VOC}',
        lang='en',
        spk_id=0,
        output=f"{wav_save_file}/en_mono.wav")

# text = "如同飞机不是飞的更高的鸟儿一样，人工智能也并不是更聪明的人。" \
#        "在大数据之前，计算机并不擅长解决需要人类智能的问题。" \
#        "但是今天这些问题换个思路就可以解决了，其核心就是变智能问题为数据问题。" \
#        "由此，全世界开始了新的一个人工智能的时代，人们开始意识到数据的重要性。" \
#        "随着技术的不断发展和计算能力的提升，人工智能正以前所未有的速度渗透到各个领域。"
# tic = time.time()
# print(tic)
# tts_zh(text)
# toc = time.time()
# print(toc - tic)

# text = "Just as airplanes are not higher birds, artificial intelligence is not smarter people." \
#        " Before big data, computers were not very good at solving problems that required human intelligence." \
#        " But today, these problems can be solved by changing the way of thinking." \
#        " The core is to change the intelligence problem into a data problem." \
#        " As a result, the world has begun a new era of artificial intelligence, and people have begun to realize the importance of data." \
#        " With the continuous development of technology and the improvement of computing power," \
#        " artificial intelligence is penetrating into various fields at an unprecedented speed."
# tic = time.time()
# print(tic)
# tts_en(text)
# toc = time.time()
# print(toc - tic)