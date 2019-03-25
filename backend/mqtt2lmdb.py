#!/usr/bin/env python -B
# -*- coding: utf-8 -*-

# Subscribe from MQTT and store in LMDB. On every message we receive
# check if record's TTL has expired, and if so, delete it; this takes
# care of vehicles which no longer report to us.

import lmdb
import paho.mqtt.client as paho   # pip install paho-mqtt
import json
import time
import sys
import ssl
import socket
import os

__author__    = "Jan-Piet Mens <jp()mens.de>"
__copyright__ = "Copyright 2019 Jan-Piet Mens"

DB_PATH         = '../db'
topic_branch    = "owntracks/zabb/+"
# topic_branch    = "owntracks/zabb/maronea"
TTL = 3600
USE_SPARSE_FILES = sys.platform != 'darwin'

def open_env():
    return lmdb.open(DB_PATH,
        map_size=1048576 * 1024,
        metasync=False,
        sync=False,
        map_async=True,
        writemap=USE_SPARSE_FILES)

def show_all():
    print("* SHOW")
    for key, value in txn.cursor():
        print(" ", key, value)

def store(env, key, val):
    with env.begin(write=True, buffers=False) as txn:
        try:
            txn.put(key.encode('utf8'), val.encode('utf8'))
        except:
            raise

def remove(delete_key):
    with env.begin(write=True, buffers=False) as txn:
        n = 1
        for key, value in txn.cursor():
            if key == delete_key:
                print("   ", n, key, value)
                n = n + 1
                txn.delete(key)

def cleaner(env):
    now = int(time.time())

    with env.begin(write=True) as txn:
        for key, val in txn.cursor():
            try:
                data = json.loads(val)
            except:
                print("Cannot decode ", val)
                txn.delete(key)
                continue

            tst = int(data["tst"])
            if tst + int(TTL) < now:
                print("---> DELETE ", (now - tst + TTL), key.decode('utf-8'), val.decode('utf-8'))
                try:
                    txn.delete(key)
                except:
                    raise
        env.sync(True)

def on_log(mosq, userdata, level, string):
    print(level, string, file=sys.stderr)

def on_connect(mosq, userdata, flags, rc):
    print("Connected to MQTT: rc: {0}".format(rc))
    mqttc.subscribe(topic_branch, 0)

def on_message(mosq, userdata, msg):
    # print("%s (qos=%s, r=%s) %s" % (msg.topic, str(msg.qos), msg.retain, str(msg.payload)))
    lmdb_env = userdata

    topic_parts = msg.topic.split("/")

    try:
        data = json.loads(msg.payload)
    except:
        return

    if "_type" in data and data["_type"] == "location":
        key = topic_parts[2]
        del(data["_type"])
        data["imei"] = topic_parts[2]

        # ensure tst in data
        if "tst" not in data:
            data["tst"] = int(time.time())

        # we may be getting an older, retained message. If its tst is
        # not fresh, don't store it.

        if int(data["tst"]) + int(TTL) >= int(time.time()):
            print("storing ", key)
            store(lmdb_env, key, json.dumps(data))

        cleaner(lmdb_env)


lmdb_env = open_env()

mqttc = paho.Client("mqtt2lmdb", clean_session=True, userdata=lmdb_env)
mqttc.on_connect = on_connect
mqttc.on_message = on_message
mqttc.on_log = on_log

mqttc.connect("localhost", 1883, 60)

while True:
    try:
        mqttc.loop_forever()
    except socket.error:
        print("MQTT server disconnected; sleeping")
        time.sleep(5)
    except KeyboardInterrupt:
        mqttc.disconnect()
        break
    except:
        raise

lmdb_env.sync(True)
lmdb_env.close()
