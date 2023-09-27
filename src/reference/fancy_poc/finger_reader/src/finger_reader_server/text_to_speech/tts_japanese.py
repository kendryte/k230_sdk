import torch
import time
from text_to_speech import commons
from text_to_speech import utils
from text_to_speech import ONNXVITS_infer
from text_to_speech.text import text_to_sequence
import scipy.io.wavfile as wavf
import warnings
warnings.filterwarnings('ignore')


def get_text(text, hps):
    text_norm = text_to_sequence(text, hps.symbols, hps.data.text_cleaners)
    if hps.data.add_blank:
        text_norm = commons.intersperse(text_norm, 0)
    text_norm = torch.LongTensor(text_norm)
    return text_norm


def tts_jp(text, wav_save_file):
    hps = utils.get_hparams_from_file("configs/japanese/uma87.json")

    net_g = ONNXVITS_infer.SynthesizerTrn(
        len(hps.symbols),
        hps.data.filter_length // 2 + 1,
        hps.train.segment_size // hps.data.hop_length,
        n_speakers=hps.data.n_speakers,
        ONNX_dir="ONNX_net/G_jp/",
        **hps.model)
    _ = net_g.eval()

    _ = utils.load_checkpoint("pretrained_models/japanese/G_jp.pth", net_g)

    text1 = get_text(text, hps)
    stn_tst = text1
    with torch.no_grad():
        x_tst = stn_tst.unsqueeze(0)
        x_tst_lengths = torch.LongTensor([stn_tst.size(0)])
        sid = torch.LongTensor([0])
        audio = net_g.infer(x_tst, x_tst_lengths, sid=sid, noise_scale=.667, noise_scale_w=0.8, length_scale=1.0)[0][
            0, 0].data.cpu().float().numpy()
    del stn_tst, x_tst, x_tst_lengths, sid

    wavf.write(f"{wav_save_file}/jp_mono.wav", hps.data.sampling_rate, audio)


if __name__ == '__main__':
    text = "飛行機が高等な鳥ではないのと同じように、人工知能も賢い人間ではありません。" \
           "ビッグデータが登場する以前、コンピューターは人間の知性を必要とする問題を解決するのがあまり得意ではありませんでした。" \
           "しかし今日では、これらの問題は考え方を変えることで解決でき、その核心はインテリジェンスの問題をデータの問題に変えることです。"
    tic = time.time()
    print(tic)
    tts_jp(text, "./output_wav")
    toc = time.time()
    print(toc - tic)