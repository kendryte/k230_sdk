from datetime import datetime, timedelta

class ExpiredDict(dict):
    def __init__(self, expires_in_seconds):
        super().__init__()
        self.expires_in_seconds = expires_in_seconds

    def __getitem__(self, key):
        value, expiry_time = super().__getitem__(key)
        if datetime.now() > expiry_time:
            del self[key]
            raise KeyError("expired {}".format(key))
        self.__setitem__(key, value)
        return value

    def __setitem__(self, key, value):
        expiry_time = datetime.now() + timedelta(seconds=self.expires_in_seconds)
        super().__setitem__(key, (value, expiry_time))
    def get(self, key, default=None):
        try:
            return self[key]
        except KeyError:
            return default