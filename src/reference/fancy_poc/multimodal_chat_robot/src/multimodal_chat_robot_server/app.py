# encoding:utf-8

import config
from channel import channel_factory
from common.log import logger


if __name__ == '__main__':
    try:
        config.load_config()
        channel = channel_factory.create_channel("Client_Server")
        channel.startup()
    except Exception as e:
        logger.error("App startup failed!")
        logger.exception(e)
