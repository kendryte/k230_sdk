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

        from bot.chatgpt.chat_gpt_bot_stream import ChatGPTBot
        return ChatGPTBot()

    raise RuntimeError
