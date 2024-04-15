import os
import pygame
import asyncio
import edge_tts

TEXT = ""


def play_audio(file_path):
    pygame.mixer.init()
    pygame.mixer.music.load(file_path)
    pygame.mixer.music.play()
    while pygame.mixer.music.get_busy():
        pass
    pygame.mixer.quit()


async def _main() -> None:
    communicate = edge_tts.Communicate(TEXT, "zh-CN-XiaoxiaoNeural")
    await communicate.save("reminder.mp3")
    play_audio("reminder.mp3")
    os.remove("reminder.mp3")


def extract_info(people_list, target_name):
    for person in people_list:
        if person['name'] == target_name:
            return person['age'], person['gender']
    return None, None


def say_to_guest(all_json, words):
    global TEXT
    TEXT = words
    asyncio.run(_main())
    print(words)
    return all_json, words


def say_to_familymembers(all_json, words):
    global TEXT
    TEXT = words
    asyncio.run(_main())
    print(words)
    return all_json, words


def say_to_stranger(all_json, words):
    global TEXT
    TEXT = words
    asyncio.run(_main())
    print(words)
    return all_json, words


def just_tts(contents):
    txt = contents['contents']
    if "您好" in txt:
        play_audio("audio_file/reminder1_nihao.mp3")
    elif "请正视" in txt:
        play_audio("audio_file/reminder2.mp3")
    else:
        play_audio("audio_file/reminder3.mp3")


