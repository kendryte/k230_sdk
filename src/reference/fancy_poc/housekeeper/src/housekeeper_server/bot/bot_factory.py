"""
channel factory
"""
def create_bot(bot_type):
    if bot_type == 'chatGPT':
        from bot.llm_stream import ChatGPTBot
        return ChatGPTBot()
    raise RuntimeError
