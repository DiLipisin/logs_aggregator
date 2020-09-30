import json
import os
import shutil


class Entry:
    def __init__(self, ts_fact, fact_name, actor_id, props):
        self.ts_fact = ts_fact
        self.fact_name = fact_name
        self.actor_id = actor_id
        assert len(props) == 10
        self.props = props

    def __str__(self):
        res_dict = {"ts_fact": self.ts_fact, "fact_name": self.fact_name, "actor_id": self.actor_id, "props": {}}
        for i in range(10):
            res_dict["props"]["prop" + str(i+1)] = self.props[i]
        return str(json.dumps(res_dict))


def props_comparator(i):
    return str(i["props"])


def order_dict(dictionary):
    result = {}
    for k, v in sorted(dictionary.items()):
        if isinstance(v, dict):
            result[k] = order_dict(v)
        elif isinstance(v, list):
            v.sort(key=props_comparator)
            result[k] = v
        else:
            result[k] = v
    return result


def make_dir(dir_path):
    if os.path.exists(dir_path):
        shutil.rmtree(dir_path)
        os.makedirs(dir_path)
    else:
        os.makedirs(dir_path)


def remove_dir(dir_path):
    if os.path.exists(dir_path):
        shutil.rmtree(dir_path)
