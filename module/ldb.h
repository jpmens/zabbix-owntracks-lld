/*
 * Copyright (C) 2019 Jan-Piet Mens <jp@mens.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
void db_begin(struct ldb *lm);
void db_close(struct ldb *);
JsonNode *db_getnext(struct ldb *);
JsonNode *db_get(struct ldb *, char *key);

#endif
