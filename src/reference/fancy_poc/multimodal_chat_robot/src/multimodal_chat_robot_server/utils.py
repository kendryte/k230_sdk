from diffusers import StableDiffusionPipeline, DPMSolverMultistepScheduler,AudioLDMPipeline, DDIMScheduler
import re,random
import struct
import numpy
def move_dict_with_role_to_penultimate(lst):
    if len(lst) < 6:
        return lst
    role_dict = None
    for i, d in enumerate(lst):
        if d.get('role') == 'system':
            role_dict = lst.pop(i)
            break
    if role_dict:
        lst.insert(-5, role_dict)
    return lst

def generator_image(prompt,client_socket):

    def stable_diffusion_callback(progress_i, progress_t, inference_lantence):
        progress = int((progress_i + 1) / 26 * 100)
        formatted_progress = "{}%".format(progress)
        content = "小楠正在努力绘制: " + formatted_progress + "\r\n"
        response = content.encode('utf-8')
        # 添加消息头，指定消息体的长度
        response_header = struct.pack('>I', len(response))
        client_socket.sendall(response_header + response)

    if "cartoon" in prompt or "anime" in prompt:
        if random.random() < 0.6:
            pipeline = StableDiffusionPipeline.from_pretrained(
                r"model_sd\dreamlilke-anime-v1.0")
        else:
            pipeline = StableDiffusionPipeline.from_pretrained(
                r"model_sd\DreamShaper")
        if random.random() < 0.5:
            prompt = prompt.replace("cartoon style", "").replace(
                "cartoon-style", "").replace("cartoon_style", "").replace("cartoon",
                                                                          "")
            prompt = prompt.replace("anime style", "").replace(
                "anime-style", "").replace("anime_style", "").replace("anime", "")
    else:
        p = random.random()
        if p < 0.6:
            pipeline = StableDiffusionPipeline.from_pretrained(
                r"model_sd\Realistic_Vision_V2.0")
        elif p > 0.8:
            pipeline = StableDiffusionPipeline.from_pretrained(
                r"model_sd\dreamlilke-photoreal-v2.0")
        else:
            pipeline = StableDiffusionPipeline.from_pretrained(
                r"model_sd\Deliberate")
    pipeline.to("cuda")
    pipeline.scheduler = DPMSolverMultistepScheduler.from_config(
        pipeline.scheduler.config, clip_sample=False)
    pattern = r"(?<=\{i_g:).*?(?=\})"
    matches = re.search(pattern, prompt).group(0)
    prompt_ = "best quality, masterpiece, illustration, finely detail, highres, 8k wallpaper, " + (
        matches)
    negative_prompt = "worst quality, low quality, normal quality, lowres, multiple eyes, bad eyes, nsfw, bad anatomy, bad hands, text, error, missing fingers,extra digit, fewer digits, signature, watermark, username, mutated hands and fingers, title,deformed, bad anatomy, disfigured, poorly drawn face, mutation, mutated, extra limb, ugly, poorly drawn hands, missing limb, floating limbs, disconnected limbs, malformed hands, out of focus, long neck, long body,lowres, bad anatomy,  bad hands, text, error,  extra digit, fewer digits, cropped,  artifacts, signature, watermark, username, blurry, missing arms, long neck, humpbacked, bad feet"
    image = pipeline(prompt_, num_inference_steps=26, guidance_scale=7.5, height=512,
                     width=512,
                     negative_prompt=negative_prompt,
                     callback=stable_diffusion_callback).images[0]
    return image

def generator_audio(prompt_task,client_socket):

    def audio_diffusion_callback(progress_i, progress_t, inference_lantence):
        progress_i = progress_i + 1
        if progress_i % 5 == 0:
            progress = int(progress_i / 50 * 100)
            formatted_progress = "{}%".format(progress)
            content = "小楠正在努力制作音乐: " + formatted_progress + "\r\n"
            response = content.encode('utf-8')
            # 添加消息头，指定消息体的长度
            response_header = struct.pack('>I', len(response))
            client_socket.sendall(response_header + response)
    pattern = r"(?<=\{m_g:).*?(?=\})"
    matches = re.search(pattern, prompt_task).group(0)
    pipeline = AudioLDMPipeline.from_pretrained(
        r"F:\chatrobot\chatgptrobot-s\model_sd\audioldm-m-full")
    pipeline.to("cuda")
    negative_prompt = "low quality, average quality,loud noise"
    pipeline.scheduler = DDIMScheduler.from_config(pipeline.scheduler.config)
    print(matches)
    audio_all = \
        pipeline("high quality,clear,beautiful," + matches,
                 negative_prompt=negative_prompt,
                 num_inference_steps=50,
                 audio_length_in_s=20, guidance_scale=3,
                 num_waveforms_per_prompt=1,
                 callback=audio_diffusion_callback).audios[0]
    audio = (audio_all * 32767).astype(numpy.int16)
    audio_two = numpy.zeros((len(audio), 2)).astype(numpy.int16)
    audio_two[:, 0] = audio
    audio_two[:, 1] = audio
    return audio_two