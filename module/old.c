#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
// #include "module.h"
#include <time.h>
#include "json.h"
#include "lm-cache.h"

#define DB_PATH "../db"

#define MAXBUF	8192	/* Hack */

/*
 * dbname is an named LMDB database; may be NULL.
 */

struct lmcache *db_open(char *path, char *dbname, int rdonly)
{
	MDB_txn *txn = NULL;
	int rc;
	unsigned int flags = 0, dbiflags = 0, perms = 0664;
	struct lmcache *lm;

	if ((lm = malloc(sizeof (struct lmcache))) == NULL)
		return (NULL);

	memset(lm, 0, sizeof(struct lmcache));

	if (rdonly) {
		flags |= MDB_RDONLY;
		perms = 0444;
		perms = 0664;
	} else {
		dbiflags = MDB_CREATE;
	}

	rc = mdb_env_create(&lm->env);
	if (rc != 0) {
		fprintf(stderr, "db_open: mdb_env_create: %s", mdb_strerror(rc));
		free(lm);
		return (NULL);
	}

	mdb_env_set_mapsize(lm->env, LMDB_DB_SIZE);

	rc = mdb_env_set_maxdbs(lm->env, 10);
	if (rc != 0) {
		fprintf(stderr, "db_open: mdb_env_set_maxdbs%s", mdb_strerror(rc));
		free(lm);
		return (NULL);
	}

	rc = mdb_env_open(lm->env, path, flags, perms);
	if (rc != 0) {
		fprintf(stderr, "db_open: mdb_env_open: %s", mdb_strerror(rc));
		free(lm);
		return (NULL);
	}

	/* Open a pseudo TX so that we can open DBI */

	mdb_txn_begin(lm->env, NULL, flags, &txn);
	if (rc != 0) {
		fprintf(stderr, "db_open: mdb_txn_begin: %s", mdb_strerror(rc));
		mdb_env_close(lm->env);
		free(lm);
		return (NULL);
	}

	rc = mdb_dbi_open(txn, dbname, dbiflags, &lm->dbi);
	if (rc != 0) {
		fprintf(stderr, "db_open: mdb_dbi_open for `%s': %s", dbname, mdb_strerror(rc));
		mdb_txn_abort(txn);
		mdb_env_close(lm->env);
		free(lm);
		return (NULL);
	}

	rc = mdb_txn_commit(txn);
	if (rc != 0) {
		fprintf(stderr, "db_open: mmit after open %s", mdb_strerror(rc));
		mdb_env_close(lm->env);
		free(lm);
		return (NULL);
	}

	return (lm);
}

void db_close(struct lmcache *lm)
{
	if (lm == NULL)
		return;

	mdb_env_close(lm->env);
	free(lm);
}

void db_enum(char *path, JsonNode *array)
{
	struct lmcache *lm;
	MDB_val key, data;
	MDB_txn *txn;
	MDB_cursor *cursor;
	int rc;
	JsonNode *json, *o, *j;

	if ((lm = db_open(path, NULL, true)) == NULL) {
		fprintf(stderr, "Cannot open lmdb at %s\n", path);
		return;
	}

	key.mv_size = 0;
	key.mv_data = NULL;

	rc = mdb_txn_begin(lm->env, NULL, MDB_RDONLY, &txn);
	if (rc) {
		fprintf(stderr, "db_enum: mdb_txn_begin: (%d) %s", rc, mdb_strerror(rc));
		db_close(lm);
		return;
	}

	rc = mdb_cursor_open(txn, lm->dbi, &cursor);

	while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0) {
		char buf[MAXBUF + 1], *plate = "UN-KNOWN", *imei = "0000000";
		size_t buflen;
		long speed = 0L;

		if ((buflen = data.mv_size) > MAXBUF)
			continue;

		// printf("%*.*s %*.*s\n",
		// 	(int)key.mv_size, (int)key.mv_size, (char *)key.mv_data,
		// 	(int)data.mv_size, (int)data.mv_size, (char *)data.mv_data);

		memcpy(buf, (char *)data.mv_data, buflen);
		buf[buflen] = 0;

		if ((json = json_decode(buf)) != NULL) {
			if ((j = json_find_member(json, "imei")) != NULL)
				imei = j->string_;
			if ((j = json_find_member(json, "vel")) != NULL)
				speed = j->number_;
			if ((j = json_find_member(json, "plate")) != NULL)
				plate = j->string_;
			else plate = imei;

			o = json_mkobject();
			json_append_member(o, "{#OT.IMEI}",	json_mkstring(imei));
			json_append_member(o, "{#OT.PLATE}",	json_mkstring(plate));
			json_append_member(o, "{#OT.VEL}",	json_mknumber(speed));
			json_append_element(array, o);

			json_delete(json);
		}
	}
	mdb_cursor_close(cursor);
	mdb_txn_commit(txn);
	db_close(lm);
}


// static int owntracks_lld(AGENT_REQUEST *request, AGENT_RESULT *result)
static int owntracks_lld(void *nil)
{
	char *js;
	JsonNode *data, *list;

	data = json_mkobject();
	list = json_mkarray();

	db_enum(DB_PATH, list);

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
