#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "json.h"
#include "ldb.h"
#ifdef TESTING
# define AGENT_REQUEST char *
# define AGENT_RESULT  char *
# define SET_MSG_RESULT(res, val)	*res = val
# define SET_STR_RESULT(res, val)	*res = val
# define SYSINFO_RET_OK			(0)
# define SYSINFO_RET_FAIL		(1)
#else
# include "module.h"
#endif

#define DB_PATH "../db"

/*
 *                       _                  _
 *    _____      ___ __ | |_ _ __ __ _  ___| | _____
 *   / _ \ \ /\ / / '_ \| __| '__/ _` |/ __| |/ / __|
 *  | (_) \ V  V /| | | | |_| | | (_| | (__|   <\__ \
 *   \___/ \_/\_/ |_| |_|\__|_|  \__,_|\___|_|\_\___/
 *
 */

/* global database handle */
static struct ldb *db;

#ifndef TESTING

/* store item processing timeout as set dynamically below */
static int item_timeout_seconds = 0;

/* return the version number of this module interface. */
int zbx_module_api_version(void)
{
	return (ZBX_MODULE_API_VERSION);
}

/* if the module exports this function then it will be used to specify timeout
 * settings from the Zabbix configuration file. */
void zbx_module_item_timeout(int timeout)
{
	item_timeout_seconds = timeout;
}

static int owntracks_ping(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	SET_UI64_RESULT(result, 42);

	return (SYSINFO_RET_OK);
}

/* zabbix_get -k owntracks.vel[IMEI] to find velocity */
static int owntracks_vel(AGENT_REQUEST *request, AGENT_RESULT *result)
{
        char *imei;
        unsigned long retval = 0L;
	JsonNode *json, *j;

        if (request->nparam != 1) {
                SET_MSG_RESULT(result, strdup("Expecting 1 parameter with IMEI"));
                return (SYSINFO_RET_FAIL);
        }

        imei = get_rparam(request, 0);
	if ((json = db_get(db, imei)) == NULL) {
		SET_MSG_RESULT(result, strdup("no such IMEI value"));
			return (SYSINFO_RET_FAIL);
	}

	if ((j = json_find_member(json, "vel")) != NULL) {
		retval = j->number_;
	}

	json_delete(json);

	SET_UI64_RESULT(result, retval);

        return (SYSINFO_RET_OK);
}
#endif /* !TESTING */

static int owntracks_lld(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	JsonNode *obj = json_mkobject(), *list = json_mkarray(), *j;
	char *js;

	db_begin(db);
	while ((j = db_getnext(db)) != NULL) {
		json_append_element(list, j);
	}

	json_append_member(obj, "data", list);

	if ((js = json_stringify(obj, "  ")) == NULL) {
		json_delete(obj);
		SET_MSG_RESULT(result, strdup("Cannot stringify JSON."));
		return (SYSINFO_RET_FAIL);
	}

	SET_STR_RESULT(result, js);		/* Pass result string to Zabbix.
						 * Zabbix will free() this */
	json_delete(obj);

	return (SYSINFO_RET_OK);
}

#ifndef TESTING

/* agent start-up; Zabbix will fail to load if we return ZBX_MODULE_FAIL */
int zbx_module_init(void)
{
        char *dbpath;

	if ((dbpath = getenv("ZBX_DB_PATH")) == NULL)
		dbpath = DB_PATH;

	if ((db = db_open(dbpath, NULL, true)) == NULL) {
		fprintf(stderr, "%s: cannot open database at %s\n", __FILE__, dbpath);
		return (ZBX_MODULE_FAIL);
	}

        return (ZBX_MODULE_OK);
}

/* agent is shutting down */
int zbx_module_uninit(void)
{
	if (db) {
		db_close(db);
	}
        return (ZBX_MODULE_OK);
}

static ZBX_METRIC item_keys[] =
/* ITEM.KEY             FLAG            FUNCTION         TEST PARAMETERS */
{
   { "owntracks.ping",  0,              owntracks_ping,  NULL},
   { "owntracks.lld",   0,              owntracks_lld,   NULL},
   { "owntracks.vel",   CF_HAVEPARAMS,  owntracks_vel,   "1234567890012345"},
   { NULL }
};

/* return list of item keys supported by this module */
ZBX_METRIC *zbx_module_item_list(void)
{
	return (item_keys);
}
#endif

#ifdef TESTING
int main(int argc, char **argv)
{
	char *request = NULL, *result = NULL;
	int ret;


	db = db_open(DB_PATH, NULL, true);

	ret = owntracks_lld(&request, &result);
	printf("%s\n", result);

	db_close(db);

	return (ret);
}
#endif
