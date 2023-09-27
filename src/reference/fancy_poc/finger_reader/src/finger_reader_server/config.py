# encoding:utf-8
import json
import os

config = {}

def load_config():
    global config
    config_path = "config.json"
    if not os.path.exists(config_path):
        raise Exception('配置文件不存在!')

    config_str = read_file(config_path)
    # 将json字符串反序列化为dict类型
    config = json.loads(config_str)


def get_root():
    return os.path.dirname(os.path.abspath( __file__ ))


def read_file(path):
    with open(path, mode='r', encoding='utf-8') as f:
        return f.read()


def conf():
    return config
