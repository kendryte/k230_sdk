from channel.channel import Channel
import sys
from utils import generator_image,generator_audio
import socket
import time
import struct
import soundfile
import whisper

class SocketChannel(Channel):
    def check_prefix(self, content, prefix_list):
        for prefix in prefix_list:
            if content.startswith(prefix):
                return prefix
        return None

    def startup(self):
        print("\n你好，我是嘉楠的聊天绘画机器人小楠，欢迎您和我聊天。")
        while True:
            try:
                # 创建socket对象
                self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                # 获取本地IP地址
                ip_address = socket.gethostbyname(socket.gethostname())
                print(ip_address)
                # 绑定IP地址和端口号
                self.server_socket.bind((ip_address, 8000))
                # 监听连接
                self.server_socket.listen()
                # 等待客户端连接
                print("Waiting for client connection...")
                while True:
                    client_socket, client_address = self.server_socket.accept()
                    print(f"Client {client_address[0]}:{client_address[1]} connected.")
                    super().build_reply_content("##清除记忆", context=None)
                    context = {"from_user_id": "User"}
                    print("\nPlease input your question")
                    while True:
                        try:
                            image_flag = False
                            draw_flag = False
                            prompt_task = ""
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
                            if message.startswith(b'audio'):
                                audio_data = message[5:]
                                with open('2.wav', 'wb') as f:
                                    f.write(audio_data)
                                model = whisper.load_model("large")
                                prompt = model.transcribe(r"2.wav",language='zh')["text"]
                            if message.startswith(b'/txt'):
                                ocr_data = message[4:]
                                with open('ocr.txt', 'wb') as f:
                                    f.write(ocr_data)
                            else:
                                # 处理数据
                                prompt = message.decode('utf-8')
                            prompt=prompt.replace("\r\n","")
                            print(prompt)
                            for res in super().build_reply_content(prompt, context):
                                #判断是否要画画或者生成音乐，是则提取关键任务指令，不是则正常聊天
                                if "{" in res:
                                    image_flag = True
                                    content = "小楠正在努力帮您创作，请耐心等待~"
                                    response = content.encode('utf-8')
                                    # 添加消息头，指定消息体的长度
                                    response_header = struct.pack('>I', len(response))
                                    client_socket.sendall(response_header + response)
                                if image_flag:
                                    prompt_task += res
                                else:
                                    response = res.encode('utf-8')
                                    if response == b'':
                                        continue
                                    response_header = struct.pack('>I', len(response))
                                    client_socket.sendall(response_header + response)
                                # 如果进入任务程序，则判断是绘画还是音乐
                                if "}" in res:
                                    if "i_g" in prompt_task:
                                        image=generator_image(prompt_task,client_socket)
                                        image.save("image.jpg")
                                        with open('image.jpg', 'rb') as f:
                                            image_data = f.read()
                                        message_header = struct.pack('>I', len(image_data))
                                        client_socket.sendall(message_header + image_data)
                                        draw_flag = True
                                        # break
                                    else:
                                        audio_two=generator_audio(prompt_task, client_socket)
                                        soundfile.write("audio.wav", audio_two, samplerate=16000)
                                        with open('audio.wav', 'rb') as f:
                                            audio_data = f.read()
                                        message = b'audio' + audio_data
                                        message_header = struct.pack('>I', len(message))
                                        client_socket.sendall(message_header + message)
                                        draw_flag = True

                                        # break
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
