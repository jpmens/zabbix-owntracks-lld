#ifndef _LMCACHE_H_INCLUDED_
# define _LMCACHE_H_INCLUDED_

#include <lmdb.h>

#define LMDB_DB_SIZE	((size_t)100000 * (size_t)(1024 * 1024))

struct ldb {
	MDB_env *env;
	MDB_dbi dbi;
	MDB_txn *txn;
	MDB_cursor *cursor;
	MDB_val key;
};

struct ldb *db_open(char *path, char *name, int rdonly);
void db_close(struct ldb *);
JsonNode *db_getnext(struct ldb *);

#endif
