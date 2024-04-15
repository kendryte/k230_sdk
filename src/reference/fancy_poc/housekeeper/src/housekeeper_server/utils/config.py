import json
import os
config = {}


def load_config():
    global config
    config_path = "./config.json"
    config_str = read_file(config_path)
    config = json.loads(config_str)


def read_file(path):
    with open(path, mode='r', encoding='utf-8') as f:
        return f.read()


def conf():
    return config
