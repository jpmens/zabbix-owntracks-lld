#!/bin/sh

export ZBX_DB_PATH=../db

# try ./run.sh -p

zdir=/usr/local/zabbix
zabbix_agentd=$zdir/sbin/zabbix_agentd
#zabbix_conf=$zdir/etc/zabbix_agentd.conf
zabbix_conf=./etc/zabbix_agentd.conf

$zabbix_agentd -c $zabbix_conf -f "$@"

