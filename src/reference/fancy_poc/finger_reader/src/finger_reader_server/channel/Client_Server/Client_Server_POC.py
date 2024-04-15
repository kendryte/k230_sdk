from channel.channel import Channel
from common.trans_channels import trans_mono_to_stereo
import sys
import socket
import time
import struct
import re
from text_to_speech.tts import tts
import openai

from pydub import AudioSegment

def convert_sample_rate(input_file, output_file, target_sample_rate=16000):
    audio = AudioSegment.from_file(input_file)
    audio = audio.set_frame_rate(target_sample_rate)
    audio.export(output_file, format='wav')

class SocketChannel(Channel):
    def check_prefix(self, content, prefix_list):
        for prefix in prefix_list:
            if content.startswith(prefix):
                return prefix

        return None



    def startup(self):

        context = {"from_user_id": "User"}
        print("\n你好，我是嘉楠的指读聊天机器人，欢迎您和我交互。")
        image_flag = False

        while True:
            try:
                # 创建socket对象
                self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

                # 获取本地IP地址
                ip_address = socket.gethostbyname(socket.gethostname())
                print(ip_address)
                # 绑定IP地址和端口号
                self.server_socket.bind((ip_address, 8080))

                # 监听连接
                self.server_socket.listen()

                # 等待客户端连接
                print("Waiting for client connection...")

                language = "no"
                while True:

                    client_socket, client_address = self.server_socket.accept()
                    print(f"Client {client_address[0]}:{client_address[1]} connected.")
                    super().build_reply_content("##清除记忆", context=None)

                    context = {"from_user_id": "User"}
                    print("\nPlease input your question")

                    while True:
                        try:
                            draw_flag = False
                            task_prompt = ""
                            message_header = client_socket.recv(4)
                            if not message_header:
                                break
                            message_length = struct.unpack('>I', message_header)[0]
                            # 根据消息体长度接收完整数据
                            message = b''
                            while len(message) < message_length:
                                data = client_socket.recv(message_length - len(message))
                                if not data:
                                    break
                                message += data
                            if not message:
                                break

                            if message.startswith(b'/txt'):
                                ocr_data = message[4:]
                                with open('ocr.txt', 'wb') as f:
                                    f.write(ocr_data)
                                with open('ocr.txt', 'r',encoding='utf-8') as f:
                                    prompt = f.read()
                                    # 结合了所有输入的内容
                                    print(prompt)

                                user_input = prompt
                                # xx,续写、对话、问答、解释、没有，对应的标志分别是xx、dh、wd、js、no
                                dict_action = {"xx": "续写下列话语：", "dh": "", "wd": "问题：", "js": "帮我解释一下：",
                                               "no": None}

                                system_action = {"xx": [
                                                        {
                                                         "role": "system",
                                                         "content": "你是一个续写型机器人，会对用户输入进行续写，续写的返回内容一定要以用户输入为开头，续写字数一定不要超过70字。"
                                                        },
                                                        {"role": "user",
                                                          "content": "续写下列话语:你好吗今天，今天天气很热，我们都在吃雪糕"},
                                                        {"role": "assistant",
                                                         "content": "你好吗今天，今天天气很热，我们都在吃雪糕,在享受清凉的雪糕呢。尤其是那种口感丝滑、奶香浓郁的巧克力味道，简直是夏天的救星。"},
                                                        {"role": "user",
                                                         "content": "续写下列话语:其结合了所有输入"},
                                                        {"role": "assistant",
                                                         "content": "其结合了所有输入的数据，并运用先进的算法进行分析和处理。通过这种方式，它能够快速地提供准确的结果和有用的信息。同时，它还能够不断学习和优化自身的功能，以适应不断变化的需求。总而言之，它是一款功能强大且智能的工具，为用户提供了全面的解决方案。"},
                                                        ],
                                                 "dh": [{"role": "system",
                                                         "content": "你是一个对话机器人，以幽默风趣的性格来回复用户，回复不超过100字。"}],
                                                 "wd": [{"role": "system",
                                                         "content": "你需要回答提出的问题，回答时使用用户提供的语言，回答的句子长度最长为100字。"}],
                                                 "js": [{"role": "system",
                                                         "content": "你需要解释用户指定的事情，回答时使用用户提供的语言，回复的句子长度最长为100字。"}]}


                                action = dict_action['xx']
                                if action is not None:
                                    messages = system_action['xx']
                                    messages.append({"role": "user", "content": action + prompt})
                                    completion = openai.ChatCompletion.create(
                                        model="gpt-3.5-turbo-16k-0613",
                                        messages=messages,
                                        temperature=0.7,  # 值在[0,1]之间，越大表示回复越具有不确定性
                                        max_tokens=1000,  # 回复最大的字符数
                                        top_p=1,
                                        frequency_penalty=0,  # [-2,2]之间，该值越大则更倾向于产生不同的内容
                                        presence_penalty=0.0,
                                    )
                                    audio_txt = completion.choices[0].message["content"]
                                else:
                                    audio_txt = prompt

                                # 用户指定的音频生成语言：’zh‘, 'en', 'jp'
                                print(audio_txt)


                                # 添加消息头，指定消息体的长度
                                xuxie_txt = audio_txt.encode('utf-8')
                                response_header = struct.pack('>I', len(xuxie_txt))
                                client_socket.sendall(response_header + xuxie_txt)
                                output_language = 'zh'
                                # tts
                                wav_save_file = 'text_to_speech/output_wav'
                                mono_wav_path, stereo_wav_name = tts(audio_txt, wav_save_file, output_language=output_language)
                                stereo_wav_path = wav_save_file + '/' + stereo_wav_name

                                target_sample_rate = 16000  # 目标采样率

                                convert_sample_rate(mono_wav_path, mono_wav_path, target_sample_rate)

                                # 转换为双声道
                                trans_mono_to_stereo(mono_wav_path, stereo_wav_path)

                                with open(stereo_wav_path, 'rb') as f:
                                    audio_data = f.read()


                                message = b'audio' + audio_data
                                # 添加消息头，指定消息体的长度
                                message_header = struct.pack('>I', len(message))

                                # 发送消息头和消息体
                                client_socket.sendall(message_header + message)
                                continue

                            else:
                                # 处理数据
                                prompt = message.decode('utf-8')

                            prompt=prompt.replace("\r\n","")
                            print(prompt)

                            for res in super().build_reply_content(prompt, context):
                                print(res)
                                if "{" in res:
                                    image_flag = True

                                    content = "小楠正在与您交互，请耐心等待~"
                                    response = content.encode('utf-8')
                                    # 添加消息头，指定消息体的长度
                                    response_header = struct.pack('>I', len(response))
                                    client_socket.sendall(response_header + response)
                                    draw_flag = True
                                if image_flag:
                                    task_prompt += res
                                else:
                                    response = res.encode('utf-8')
                                    if response == b'':
                                        continue
                                    # 添加消息头，指定消息体的长度
                                    response_header = struct.pack('>I', len(response))
                                    client_socket.sendall(response_header + response)
                                if "}" in res:
                                    image_flag = False
                                    pattern = r"(?<=\{task:).*?(?=\})"
                                    matches = re.search(pattern, task_prompt).group(0)
                                    language=matches[-2:]
                                    message = b'/Task' + matches[:3].encode('utf-8')
                                    # 添加消息头，指定消息体的长度
                                    message_header = struct.pack('>I', len(message))
                                    client_socket.sendall(message_header + message)

                            if draw_flag:
                                pass
                            else:
                                client_socket.sendall(struct.pack('>I', 3) + b'END')
                        except KeyboardInterrupt:
                            print("\nExiting...")
                            sys.exit()

                    client_socket.close()

            except Exception as e:
                print(f"Exception occurred: {e}")
                print("Restarting server in 1 seconds...")
                time.sleep(1)

    def get_input(self, prompt):
        """
        Multi-line input function
        """
        message = self.server_socket.recv(1024)
        return message
