from zabbix_api import ZabbixAPI    # https://pypi.org/project/zabbix-api/
import json

zapi = ZabbixAPI(server="http://server/zabbix/")
zapi.login("Admin", "zabbix")

res = zapi.item.create({
    "name"          : "The pong",
    "key_"          : "owntracks.ping",
    "hostid"        : 10268,
    "interfaceid"   : "11",
    "type"          : 0,    # Zabbix agent
    "value_type"    : 3,    # numeric, unsigned
    "delay"         : "60s",
    })

print(json.dumps(res, indent=4))
