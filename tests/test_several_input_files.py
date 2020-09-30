import json
import subprocess

from datetime import datetime
from random import randrange, choice
from shutil import copyfile
from time import monotonic

import utils


def test_ok():
    bin_path = '../cmake-build-debug/bin/logs'
    logs_dir = '/tmp/log_dir'
    files_number = 10
    threads_number = 10
    result_dir = '/tmp/res'
    test_result_dir = '/tmp/test_res'
    file_lines_number = 100000

    fact_names = ['fact_name' + str(i) for i in range(1, 100)]
    utils.make_dir(logs_dir)

    timer = monotonic()

    result = {}
    for i in range(file_lines_number):
        entry = utils.Entry(
            randrange(1600560000, 1601424000),
            choice(fact_names),
            111222,
            [randrange(1, 1000) for i in range(10)]
        )
        with open(logs_dir + '/file1.log', 'a') as file:
            file.write(str(entry) + '\n')

        dt = str(datetime.utcfromtimestamp(entry.ts_fact).date())
        if dt not in result:
            result[dt] = {}
        if entry.fact_name not in result[dt]:
            result[dt][entry.fact_name] = []
        result[dt][entry.fact_name].append({'props': entry.props, 'count': files_number})

    for i in range(2, files_number + 1):
        copyfile(logs_dir + '/file1.log', logs_dir + '/file' + str(i) + '.log')

    print('generation time:', monotonic() - timer)
    timer = monotonic()

    utils.make_dir(test_result_dir)
    with open(test_result_dir + '/agr.txt', 'w') as file:
        file.write(json.dumps(utils.order_dict(result)))

    print('result writing time:', monotonic() - timer)
    timer = monotonic()

    ret = subprocess.run([bin_path, logs_dir, str(files_number), str(threads_number), result_dir])
    assert ret.returncode == 0, ret.stderr

    print('application running time:', monotonic() - timer)

    with open(result_dir + '/agr.txt') as file:
        res_content = file.read()
    with open(test_result_dir + '/agr.txt', 'r') as file:
        test_res_content = file.read()

    assert res_content == test_res_content

    utils.remove_dir(logs_dir)
    utils.remove_dir(result_dir)
    utils.remove_dir(test_result_dir)
