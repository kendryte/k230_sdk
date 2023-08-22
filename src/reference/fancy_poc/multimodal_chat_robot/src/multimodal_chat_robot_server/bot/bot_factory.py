"""
channel factory
"""
from common import const


def create_bot(bot_type):
    """
    create a channel instance
    :param channel_type: channel type code
    :return: channel instance
    """
    if bot_type == const.CHATGPT:
        # ChatGPT 网页端web接口
        from bot.chat_gpt_bot_stream import ChatGPTBot
        return ChatGPTBot()
    raise RuntimeError
