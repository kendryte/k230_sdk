"""
channel factory
"""

def create_channel(channel_type):
    """
    create a channel instance
    :param channel_type: channel type code
    :return: channel instance
    """
    if channel_type == 'Client_Server':
        from channel.Client_Server.Client_Server_K230 import SocketChannel
        return SocketChannel()
    raise RuntimeError
