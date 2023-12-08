# encoding:utf-8
from bot.bot import Bot
from config import conf, load_config
from common.log import logger
from common.expired_dict import ExpiredDict
from utils import move_dict_with_role_to_penultimate
import openai
import time
import json

if conf().get('expires_in_seconds'):
    all_sessions = ExpiredDict(conf().get('expires_in_seconds'))
else:
    all_sessions = dict()
# OpenAI对话模型API (可用)
class ChatGPTBot(Bot):
    def __init__(self):
        openai.api_key = conf().get('open_ai_api_key')
        if conf().get('open_ai_api_base'):
            openai.api_base = conf().get('open_ai_api_base')
        proxy = conf().get('proxy')
        if proxy:
            openai.proxy = proxy

    def reply(self, query, context=None):
        if query == "##清除记忆":
            Session.clear_all_session()
        else:
            if not context or not context.get('type') or context.get('type') == 'TEXT':
                session_id = context.get('session_id') or context.get('from_user_id')
                if query == '#清除记忆':
                    Session.clear_session(session_id)
                    return '记忆已清除'
                elif query == '#清除所有':
                    Session.clear_all_session()
                    return '所有人记忆已清除'
                elif query == '#更新配置':
                    load_config()
                    return '配置已更新'
                session = Session.build_session_query(query, session_id)
                logger.debug("[OPEN_AI] session query={}".format(session))
                reply_content = self.reply_text(session, session_id, 0)
                return reply_content
            elif context.get('type', None) == 'IMAGE_CREATE':
                return self.create_img(query, 0)

    def reply_text(self, session, session_id, retry_count=0) -> dict:
        '''
               call openai's ChatCompletion to get the answer
               :param session: a conversation session
               :param session_id: session id
               :param retry_count: retry count
               :return: {}
               '''
        try:
            session = move_dict_with_role_to_penultimate(session)
            response = openai.ChatCompletion.create(
                model=conf().get("model") or "gpt-3.5-turbo",
                # model= conf().get("model") or "text-davinci-003",# 对话模型的名称
                messages=session,
                temperature=0,  # 值在[0,1]之间，越大表示回复越具有不确定性
                max_tokens=2000,  # 回复最大的字符数
                top_p=1,
                frequency_penalty=0.0,  # [-2,2]之间，该值越大则更倾向于产生不同的内容
                presence_penalty=0.0,
                stream=True
            )
            all_stream_content = ""
            for r in response:
                choices = r.get("choices")
                delta = choices[0].get("delta")
                if "content" in delta:
                    content = delta["content"]
                    all_stream_content += content
                    yield content

            Session.save_session(all_stream_content, session_id, len(all_stream_content))
            print(all_sessions)
        except openai.error.RateLimitError as e:
            # rate limit exception
            content = "提问太快啦，请休息一下再问我吧"
            yield content

        except openai.error.APIConnectionError as e:
            content = "网络连接失败"
            yield content
        except openai.error.Timeout as e:
            content = "消息接收异常"
            yield content
        except Exception as e:
            # unknown exception
            content = "问的太多啦，请休息一下再问我吧"
            yield content

class Session(object):
    @staticmethod
    def build_session_query(query, session_id):
        '''
        build query with conversation history
        e.g.  [
            {"role": "system", "content": "You are a helpful assistant."},
            {"role": "user", "content": "Who won the world series in 2020?"},
            {"role": "assistant", "content": "The Los Angeles Dodgers won the World Series in 2020."},
            {"role": "user", "content": "Where was it played?"}
        ]
        :param query: query content
        :param session_id: session id
        :return: query content with conversaction
        '''
        session = all_sessions.get(session_id, [])
        if len(session) == 0:
            system_prompt = conf().get("character_desc", "")
            demos_or_presteps = open(conf().get("task_ana_file", ""), "r", encoding="utf-8").read()
            messages = json.loads(demos_or_presteps)
            messages.insert(0, {"role": "system", "content": system_prompt})
            for message in messages:
                session.append(message)
            all_sessions[session_id] = session
        user_item = {'role': 'user', 'content': query}
        session.append(user_item)
        return session

    @staticmethod
    def save_session(answer, session_id, total_tokens):
        max_tokens = conf().get("conversation_max_tokens")
        if not max_tokens:
            # default 3000
            max_tokens = 1000
        max_tokens = int(max_tokens)

        session = all_sessions.get(session_id)
        if session:
            # append conversation
            gpt_item = {'role': 'assistant', 'content': answer}
            session.append(gpt_item)

        # discard exceed limit conversation
        Session.discard_exceed_conversation(session, max_tokens, total_tokens)

    @staticmethod
    def discard_exceed_conversation(session, max_tokens, total_tokens):
        dec_tokens = int(total_tokens)
        # logger.info("prompt tokens used={},max_tokens={}".format(used_tokens,max_tokens))
        while dec_tokens > max_tokens:
            # pop first conversation
            if len(session) > 3:
                session.pop(1)
                session.pop(1)
            else:
                break
            dec_tokens = dec_tokens - max_tokens

    @staticmethod
    def clear_session(session_id):
        all_sessions[session_id] = []

    @staticmethod
    def clear_all_session():
        all_sessions.clear()
