#ifndef DBUTILS_H_UAKYGYV2
#define DBUTILS_H_UAKYGYV2

#include "sqlite3.h"
#define MAX_FILE_NAME_LEN   2048
#define MAX_TABLES          10
#define MAX_TABLE_NAME_LEN  100
#define MAX_QUERY_LEN 10240

struct sDB
{
	char       dbFile[MAX_FILE_NAME_LEN];
	char       tables[MAX_TABLES][MAX_TABLE_NAME_LEN];
	int        tablesCnt;
	const char *table_init_query;
	sqlite3    *db;
};

struct sqlresult{
	int colnum;
	char **colname;
	char **data;
	struct sqlresult * next;
};

struct query_result{
	int total;
	struct sqlresult* result;
	struct sqlresult* tail;
};


struct sDB *db_open(const char *dbfile, const char *table_init);
int db_close(struct sDB *);
int db_query(struct sDB *, const char *query_cmd);
int db_query_call(struct sDB *,const char* query_cmd, struct query_result *query_result);
int free_result(struct query_result * query_result);

#endif /* end of include guard: DBUTILS_H_UAKYGYV2 */
