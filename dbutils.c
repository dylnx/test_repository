#include <stdio.h>
#include <assert.h>
#include "dbutils.h"



int free_result(struct query_result * query_result)
{
        struct sqlresult * temp = query_result->result;
	//释放每行元素
	while(temp)
	{
		struct sqlresult * f = temp;
		int i=0;
		for( ; i<f->colnum;i++)
		{
			free(f->colname[i]);
			free(f->data[i]);
		}
		free(f->colname);
		free(f->data);
		temp = temp->next;
	}
	//释放每行头结点
	while(temp)
	{
		struct sqlresult *f =temp;
		temp=temp->next;
		free(f);
	}
	free(query_result);
	query_result = NULL;
}

static int callback_save_result(void *callresult, int argc, char **argv, char **azColName){

	struct query_result * query_result = (struct query_result*) callresult;
	query_result->total++;
	struct sqlresult * result  = (struct sqlresult*)malloc(sizeof(struct sqlresult));
	
	if( query_result->tail == NULL)
	{
		query_result->result = result;
		query_result->tail = result;
	}
	else
	{
		query_result->tail->next = result;
		query_result->tail = result;
	}
	
	result->colnum = argc;
	result->colname = (char**)malloc(sizeof(char*)*argc);
	result->data = (char**)malloc(sizeof(char*)*argc);
	int i = 0;
	for( ; i < argc ; i++)
	{
		int len_col = strlen(azColName[i]);
		int len_dat = strlen(argv[i]);
		result->colname[i] = (char*)malloc(len_col+1);
		result->data[i] = (char*)malloc(len_dat+1);
		strcpy(result->colname[i], azColName[i]);
		strcpy(result->data[i], argv[i]);
	}
	return 0;
}

int show_result(struct query_result * result)
{
	struct  sqlresult * temp = result->result;
	while(temp)
	{
		struct sqlresult * h = temp;
  int i;
  for(i=0; i< h->colnum; i++){
    printf("%s = %s\n", h->colname[i], h->data[i] ? h->data[i] : "NULL");
  }
  printf("\n");
  temp = temp->next;
	}
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @功能  数据库打开，如果文件不存在则新建数据库
 *
 * @参数 dbfile 数据库文件名
 * @参数 table_init 如果传入NULL，则不建立表,否则根据对应的SQL语句建立表结构
 *
 * @返回值  struct sDB* 返回的数据库指针
 */
/* ----------------------------------------------------------------------------*/
struct sDB * db_open(const char *dbfile, const char *table_init)
{
	if ( dbfile == NULL ) return -1;
	sqlite3 *db;
    int rc = sqlite3_open_v2(
					dbfile, 
					&db, 
					SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
					NULL);
	if( rc != 0 ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	    return NULL;
	}
	struct sDB * sDB = (struct sDB*)malloc(sizeof(struct sDB));
	assert(sDB != NULL );
	sDB->db = db;
	strcpy(sDB->dbFile, dbfile);
	return sDB;
}

/* --------------------------------------------------------------------------*/
/**
 * @功能  关闭数据库
 *
 * @参数 sDB 数据库的指针
 *
 * @返回值   0 关闭成功
 */
/* ----------------------------------------------------------------------------*/
int db_close(struct sDB *sDB)
{
	if( sDB == NULL )
	{
		return 0;
	}
	sqlite3_close(sDB->db);
	free(sDB);
	return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @功能  查询SQL语句，并将结果打印出来
 *
 * @参数 sDB  数据库指针
 * @参数 query_cmd  查询语句
 *
 * @返回值   0 成功  -1 失败
 */
/* ----------------------------------------------------------------------------*/
int db_query(struct sDB *sDB, const char* query_cmd)
{
	if ( sDB == NULL || query_cmd == NULL ) return -1;

	char *zErrMsg = 0;
	int rc = sqlite3_exec(sDB->db, query_cmd , callback, 0, &zErrMsg);
	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return(-1);
	}
	return (0);
}


/* --------------------------------------------------------------------------*/
/**
 * @功能  查询SQL语句，保存在传入结构体中
 *
 * @参数 sDB 数据库指针
 * @参数 query_cmd 查询语句
 * @参数 query_result 查询结果
 *
 * @返回值   0 成功 -1 失败
 */
/* ----------------------------------------------------------------------------*/
int db_query_call(struct sDB *sDB, const char* query_cmd, struct query_result *query_result)
{
	if ( sDB == NULL || query_cmd == NULL || 
			query_result == NULL ) return -1;
	char *zErrMsg = 0;
	int rc = sqlite3_exec(sDB->db, query_cmd , callback_save_result, query_result, &zErrMsg);
	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);

		//释放申请的资源
                free_result(query_result);

		//释放申请的资源
		sqlite3_free(zErrMsg);
		return (-1);
	}
	return (0);
}

#if 0
int main(int argc,char *argv[])
{
    int ret;
    struct sDB * sdb;
    struct query_result *result;
    char query_cmd[] ="select * from whitelist";
    //  char query_cmd[] ="delete from whitelist";

    result = (struct query_result *)calloc(1,sizeof(struct query_result));

    sdb = db_open("whitelist.db",NULL);	
    if(NULL == sdb)
    {	
       printf("whitelist is not exit!!!\n");
	return -1;
    } 
    
    ret = db_query(sdb,query_cmd);
    if( 0 == ret)
    {
	printf("query successfully!!!\n");	
    }
    
    ret = db_query_call(sdb, query_cmd, result);
    if( ret==0 )
    {
	struct sqlresult * temp = result->result;
#if 0
	if( result.total == 1 ) return 0;
	else  return -1;
#endif
	
	while(temp)
	{
		struct sqlresult *h = temp;
		int i ;
		for(i=0;i<h->colnum;i++)
		{
			printf("%s = %s\n", h->colname[i], h->data[i]? h->data[i] : "NULL");
		}
		printf("\n");
		temp=temp->next;
	}
	
    }

    free_result(result);

    return 0;
}
#endif
