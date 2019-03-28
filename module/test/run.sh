#!/bin/sh

export ZBX_DB_PATH=db

zdir=/usr/local/zabbix/
zabbix_agentd=$zdir/sbin/zabbix_agentd
zabbix_conf=./zabbix_agentd.conf

$zabbix_agentd -c $zabbix_conf -f "$@"
