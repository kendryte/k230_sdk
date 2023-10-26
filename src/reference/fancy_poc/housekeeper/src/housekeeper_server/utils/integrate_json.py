import json


def integrate_all_json_client(dynamic_events):

    people_json = dynamic_events
    with open(f'structured_json/history.json', encoding='utf-8') as file:
        history_json = json.load(file)
    with open(f'structured_json/j_static_environment.json', encoding='utf-8') as file:
        environ_json = json.load(file)
    all_json = {'dynamic_events': {}, 'static_enviroment': {}}

    # 加载历史信息
    for person in people_json['people']:
        if person['name'] in history_json:
            for key, value in history_json[person['name']].items():
                person[key] = value

    # 合并静态环境信息
    for key, value in environ_json.items():
        if key not in all_json['static_enviroment'] and key != 'timestamp':
            all_json['static_enviroment'][key] = value

    # 合并动态事件信息
    for key, value in people_json.items():
        if key not in all_json['dynamic_events'] and key != 'timestamp':
            all_json['dynamic_events'][key] = value
    return all_json


def format_dynamic_events(lines):
    dynamic_events = {"people": []}
    try:
        person = {}
        for line in lines:
            parts = line.split()
            person[parts[0]] = ' '.join(parts[1:])
        dynamic_events["people"].append(person)
        return dynamic_events
    except Exception as e:
        print("An error occurred:", e)


def format_task(lines):
    task = {}
    try:
        for line in lines:
            parts = line.split()
            task[parts[0]] = ''.join(parts[1:])
        return task
    except Exception as e:
        print("An error occurred:", e)


def format_register_info(lines):
    register_info = {}
    try:
        for line in lines:
            parts = line.split()
            register_info[parts[0]] = ''.join(parts[1:])
        return register_info
    except Exception as e:
        print("An error occurred:", e)


def add_history(register_info):
    with open("./structured_json/history.json") as file:
        history = json.load(file)

    if register_info["name"] not in history:
        history[register_info["name"]] = {}
        history[register_info["name"]]["role"] = register_info["role"]

    with open("./structured_json/history.json", "w", encoding="utf-8") as file:
        json.dump(history, file, indent=3)
