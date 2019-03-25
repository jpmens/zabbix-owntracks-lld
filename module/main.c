#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
// #include "module.h"
#include <time.h>
#include "json.h"
#include "ldb.h"

#define DB_PATH "../db"

// static int owntracks_lld(AGENT_REQUEST *request, AGENT_RESULT *result)
static int owntracks_lld(void *nil)
{
	JsonNode *data = json_mkobject(), *list = json_mkarray(), *j;
	struct ldb *db;
	char *js;

	db = db_open(DB_PATH, NULL, true);

	while ((j = db_getnext(db)) != NULL) {
		// printf("%s\n", json_stringify(j, "  "));
		json_append_element(list, j);
	}
	db_close(db);

	json_append_member(data, "data", list);


	if ((js = json_stringify(data, "  ")) == NULL) {
		json_delete(data);
		// SET_MSG_RESULT(result, strdup("Cannot stringify JSON."));
		// return (SYSINFO_RET_FAIL);
		puts("FAIL");
	}

	fprintf(stderr, "%s\n", js);
	// SET_STR_RESULT(result, js);

	/* I don't free `js' as Zabbix core will do that for me */

	json_delete(data);

	//return (SYSINFO_RET_OK);
	return (0);
}

int main()
{
	owntracks_lld(NULL);

	return 0;
}
