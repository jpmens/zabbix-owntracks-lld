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

static int owntracks_lld(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	JsonNode *obj = json_mkobject(), *list = json_mkarray(), *j;
	struct ldb *db;
	char *js;

	db = db_open(DB_PATH, NULL, true);

	while ((j = db_getnext(db)) != NULL) {
		json_append_element(list, j);
	}
	db_close(db);

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

int main()
{
	char *request = NULL, *result = NULL;
	int ret;

	ret = owntracks_lld(&request, &result);
	printf("%s\n", result);

	return 0;
}
