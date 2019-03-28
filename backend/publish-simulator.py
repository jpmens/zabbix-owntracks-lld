#!/usr/bin/env python3 -B

import paho.mqtt.publish as mqtt  # pip install --upgrade paho-mqtt
import json
from collections import namedtuple
import time
import random
import base64
import os

Loc = namedtuple('Loc', 'imei lat lon alt vel cog plate tid')

positions = [
    Loc("516081298750081", 59.373399,  17.946690, 101,  15, 280, "25-33-XQ", "xq"),
    Loc("507957223448860", 32.851537, -80.012624,  90, 119, 110, "72-93-FQ", "fq"),
    Loc("523674523738680", 41.474304,  15.397448,  14,   0, 110, "JL-19-74", "JL"),
    Loc("867260028922633", 40.878464,  -5.562181, 341,  56, 135, "A 0815 BL", "Gn"),
    Loc("012549656802107", 48.856826,   2.292713,  43,  72, 205, "20-ER-20", "JJ"),
]

params = {
        "hostname"  : "localhost",
        "port"      : 1883,
        "qos"       : 1,
        "retain"    : True,
        "client_id" : "pub-simu",
}

for p in positions:
    topic = "owntracks/zbx/%s" % (p.imei)
    # adjust random time: five minutes ago plus a bit
    tst = int(time.time()) - 300 + random.randint(0, 120)
    tst = int(time.time())
    vel = p.vel
    # vel = 10
    data = {
        "_type"     : "location",
        "tst"       : tst,
        "lat"       : p.lat,
        "lon"       : p.lon,
        "cog"       : p.cog,
        "alt"       : p.alt,
        "vel"       : vel,
        "tid"       : p.tid,
        "batt"      : random.randint(82, 100),
        "name"     : p.plate,                   # OSM popup on last/
    }
    payload = json.dumps(data)
    mqtt.single(topic, payload,
            auth=None,
            tls=None,
            **params)



    topic = "owntracks/zbx/%s/card" % (p.imei)
    card = {
            "_type" : "card",
            "name"  : p.plate,
    }
    for ext in [ "png", "jpg" ]:
        image_file = "faces/{0}.{1}".format(p.imei, ext)
        if os.path.exists(image_file):
            img = base64.b64encode(open(image_file, "rb").read()).decode("utf-8")
            card["face"] = img
            break

    mqtt.single(topic, json.dumps(card),
        auth=None,
        tls=None,
        **params)



