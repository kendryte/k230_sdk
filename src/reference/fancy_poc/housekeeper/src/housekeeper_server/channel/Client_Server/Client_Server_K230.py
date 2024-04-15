import sys
import socket
import struct
import re
import json
import time
from channel.channel import Channel
from utils.integrate_json import integrate_all_json_client, format_dynamic_events, format_task, format_register_info, \
    add_history
from tasks import *


class SocketChannel(Channel):
    def startup(self):
        print("\n你好，我是管家小楠。")
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
                while True:
                    client_socket, client_address = self.server_socket.accept()
                    print(f"Client {client_address[0]}:{client_address[1]} connected.")
                    super().build_reply_content("##清除记忆", context=None)
                    context = {"from_user_id": "User"}
                    task_flag = False
                    while True:
                        try:
                            task_prompt = ""

                            # 接收来自客户端的消息
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

                            # 解析来自客户端的信息
                            prompt = message.decode('utf-8')
                            lines = prompt.strip().split('\n')
                            if len(lines) == 2:
                                dynamic_events = format_dynamic_events(lines)
                                prompt = dynamic_events
                            elif len(lines) == 3:
                                register_info = format_register_info(lines)
                                add_history(register_info)
                                continue
                            else:
                                tts_task = format_task(lines)
                                just_tts(tts_task)
                                continue
                            print(prompt)
                            if prompt["people"][0]["name"] == "unknown":
                                prompt["people"][0]["role"] = "stranger"
                            all_json = integrate_all_json_client(prompt)
                            prompt = f'{all_json}'
                            context['type'] = None
                            sentence = ""

                            # 利用Chatgpt生成回复并发送给客户端
                            for res in super().build_reply_content(prompt, context):

                                # 根据回复内容的格式解析回复内容
                                if "<" in res:
                                    task_flag = True
                                    content = "小楠正在执行任务！请耐心等待呀~\n"
                                    response = content.encode('utf-8')

                                    # 添加消息头，指定消息体的长度
                                    response_header = struct.pack('>I', len(response))
                                    print("长度 response_header：{}  长度 response：{}"
                                          .format(len(response_header), len(response)))
                                    client_socket.sendall(response_header + response)
                                if task_flag:
                                    task_prompt += res
                                else:
                                    if "\n" in res:
                                        sentence = ""
                                    else:
                                        sentence += res
                                if ">" in res:
                                    task_flag = False
                                    print(task_prompt)
                                    pattern = r"(?<=\<<).*?(?=\>>)"
                                    matches = re.search(pattern, task_prompt.replace("\n", ""))\
                                        .group(0).replace("[", "").replace("]", "")
                                    response = (matches + "\n").encode('utf-8')
                                    if response == b'':
                                        continue
                                    response_header = struct.pack('>I', len(response))
                                    client_socket.sendall(response_header + response)

                                    # 解析管家的任务列表
                                    # try:
                                    task_list = json.loads(f"[{matches}]")
                                    # except:
                                    #     print(matches)

                                    # 管家执行任务
                                    for task in task_list:
                                        func_name = task['task']
                                        args = task['args']
                                        function = globals().get(func_name)
                                        if function:
                                            all_json, task_string = function(all_json, **args)
                                        else:
                                            print("Function not found:", func_name)
                            client_socket.sendall(struct.pack('>I', 3) + b'END')

                        except KeyboardInterrupt:
                            print("\nExiting...")
                            sys.exit()

                    client_socket.close()

            except Exception as e:
                print(f"Exception occurred: {e}")
                print("Restarting server in 1 seconds...")
                time.sleep(1)
