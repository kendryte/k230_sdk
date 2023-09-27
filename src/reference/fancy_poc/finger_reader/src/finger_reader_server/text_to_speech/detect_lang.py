from langdetect import detect
import time


def detect_language(text):
    detect_tic = time.time()
    language_code = detect(text)
    detect_toc = time.time()
    print(f'语种检测时间为：{detect_toc - detect_tic}')
    return language_code


# text_1 = "如同飞机不是飞的更高的鸟儿一样，人工智能也并不是更聪明的人。在大数据之前，计算机并不擅长解决需要人类智能的问题。" \
#          "但是今天这些问题换个思路就可以解决了，其核心就是变智能问题为数据问题。" \
#          "由此，全世界开始了新的一个人工智能的时代，人们开始意识到数据的重要性。" \
#          "随着技术的不断发展和计算能力的提升，人工智能正以前所未有的速度渗透到各个领域。" \
#          "在医疗领域，人工智能被应用于疾病诊断和治疗方面。" \
#          "通过分析庞大的医疗数据，人工智能可以帮助医生快速准确地诊断疾病，提供个性化的治疗方案。" \
#          "这不仅大大提高了医疗效率，还能够挽救更多的生命"
#
# text_2 = "Just as airplanes are not higher birds, artificial intelligence is not smarter people." \
#          "Before big data, computers were not very good at solving problems that required human intelligence." \
#          "But today, these problems can be solved by changing the way of thinking." \
#          "The core is to change the intelligence problem into a data problem." \
#          "As a result, the world has begun a new era of artificial intelligence, and people have begun to realize the importance of data." \
#          "With the continuous development of technology and the improvement of computing power," \
#          "artificial intelligence is penetrating into various fields at an unprecedented speed."
#
# text_3 = "飛行機がもっと高い鳥ではないように、人工知能ももっと賢い人ではない。" \
#          "ビッグデータ以前に、コンピュータは人間の知能を必要とする問題を解決するのが得意ではありません。" \
#          "しかし、今日ではこれらの問題は考え方を変えれば解決できます。その核心はスマートな問題をデータ問題に変えることです。" \
#          "これにより、世界中で新たな人工知能の時代が始まり、データの重要性が認識されるようになった。" \
#          "技術の発展と計算能力の向上に伴い、人工知能はかつてない速度で各分野に浸透している。"
#
# print(detect_language(text_1))
# print(detect_language(text_2))
# print(detect_language(text_3))
