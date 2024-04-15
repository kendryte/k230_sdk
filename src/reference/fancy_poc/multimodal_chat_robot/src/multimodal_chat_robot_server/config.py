# encoding:utf-8

import json
import os
from common.log import logger

config = {}

def load_config():
    global config
    config_path = "config.json"
    config_str = read_file(config_path)
    config = json.loads(config_str)

def get_root():
    return os.path.dirname(os.path.abspath( __file__ ))

def read_file(path):
    with open(path, mode='r', encoding='utf-8') as f:
        return f.read()

def conf():
    return config
