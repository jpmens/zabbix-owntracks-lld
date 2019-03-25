#!v3/bin/python -B

import lmdb
import sys
import json

USE_SPARSE_FILES = sys.platform != 'darwin'
DB_PATH = '../db'


def open_env():
    return lmdb.open(DB_PATH,
        map_size=1048576 * 1024,
        metasync=False,
        sync=False,
        map_async=True,
        writemap=USE_SPARSE_FILES)

env = open_env()

with env.begin() as txn:
    for key, value in txn.cursor():
        k = key.decode("utf-8")
        d = json.loads(value)

        print("{0:15s} {1} {2:20s} {3}".format(k, d['tid'], d['imei'], d['tst']))

env.close()
