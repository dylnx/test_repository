#include <stdio.h>
#include "common_sz.h"
#include "dbutils.h"
#include "sqlite3.h"
#include "whitelist.h"

const char  g_wldb_file[] = "whitelist.db";

struct sDB * g_whitelist;

static pthread_mutex_t whitelist_mtx;

int WhiteListDatabaseInit()
{
       int mtx = 0;

       mtx = pthread_mutex_init(&whitelist_mtx,NULL); 
       if( 0 != mtx){
            print_log(f_sysinit,"ERROR!!!pthread_mutex_init whilte list database mutex!!!");
	    return -1;
       }

	g_whitelist = db_open(g_wldb_file, NULL);
	if( g_whitelist==NULL )
	{
		printf("%s not exist!!!\n", g_wldb_file);
                print_log(f_sysinit,"ERROR!!! %s not exist!!!\n",g_wldb_file);
		return -1;
	}

	//
	//create table whitelist 
	//( 
	//	tid as varchar(20) primary key,
	//	priv as int(2),
	//	license as varchar(20)
	//);
	return 0;
}


/* --------------------------------------------------------------------------*/
/**
 * @功能  更新白名单数据库
 *
 * @参数 data  白名单原始数据
 * @参数 count 白名单原始数据中条目数
 *
 * @返回值   
 */
/* ----------------------------------------------------------------------------*/
int RefreshWLDatabase(char *data, int count)
{
	int     i=0;
	int     ret=0;
	char    *ptr = data;
	int     wl_item_size = 4+TID_LEN+CAR_NUM_LEN;
	char    tid[20];
	char    priv_u[5];
	int     priv;
	char    license[20];
	char    *sql_cmd;
	int     sql_prefix_len = 0;
	int     sql_cmd_len =0;
	int     split_count = 200;// 每次插入200条
	sql_cmd = (char*)malloc(200*40+30);
	sprintf(sql_cmd, "delete from whitelist");

	pthread_mutex_lock(&whitelist_mtx);
	ret = db_query(g_whitelist, sql_cmd);
	printf("Table Clear %d\n", ret);
	sql_prefix_len = sprintf(sql_cmd, "insert into whitelist values ");
	for(i=0;i<count;i++)
	{
//		char temp_str[100];
//		memset(temp_str, 0x00, 100);
//		strcpy(temp_str, ptr+33*count);
//		printf(": %s\n",  temp_str);
		ptr = data+(wl_item_size*i);
		if( i!=count-1) 
			ptr[wl_item_size] = 0;
		memset(tid, 0x00, 20);
		memset(license, 0x00, 20);
		memset(priv_u, 0x00, 5);

		memcpy(tid, ptr+2, 16);
		memcpy(priv_u, ptr+18, 2);
		priv = atoi(priv_u);
		strcpy(license, ptr+20);
		
		sql_cmd_len += sprintf(sql_cmd+sql_prefix_len+sql_cmd_len, "('%s',%d, '%s'),", 
			tid,
			priv,
			license);

		if( i%200 == 0 && i!=0 )
		{
			sql_cmd[sql_prefix_len+sql_cmd_len-1] = 0;
		//	printf("%s\n", sql_cmd);
			ret = db_query(g_whitelist, sql_cmd);
			if( ret==0 )
			{
//				printf("sql update 200 success\n");
			}
			else
			{
				printf("sql update failed\n");
				pthread_mutex_unlock(&whitelist_mtx);
				free(sql_cmd);

				return -1;
			}
			memset(sql_cmd, 0x00, 200*40+30);
			sql_cmd_len =0;
			sql_prefix_len = sprintf(sql_cmd, "insert into whitelist values ");
			continue;
		}
	}
	// 如果有剩余的不足2000条，继续插入
	if( i%200!=0 && i!=0 )
	{
		sql_cmd[sql_prefix_len+sql_cmd_len-1] = 0;
//		printf("%s\n", sql_cmd);
		ret = db_query(g_whitelist, sql_cmd);
		if( ret==0 )
		{
//			printf("sql update %d success\n", i%200);
		}
		else
		{
			printf("sql update failed\n");
			pthread_mutex_unlock(&whitelist_mtx);
			free(sql_cmd);

			return -1;
		}
	}

	free(sql_cmd);
	pthread_mutex_unlock(&whitelist_mtx);
	return 0;
}


int CheckWhiteList(const char *tid,char *carnum)
{
	char sql_cmd[1024];
	struct query_result     *result;
	int                     ret;
	if(NULL == carnum){
		return -1;
	}
        
        result = (struct query_result *)calloc(1,sizeof(struct query_result));
        if( NULL == result )
        {
            printf("calloc result is NULL!!!\n");
            return -1;
        }
 	
	sprintf(sql_cmd, "select * from whitelist where tid='%s'", tid);
	pthread_mutex_lock(&whitelist_mtx);
	ret = db_query_call(g_whitelist, sql_cmd, result);

	if( ret==0 )
	{
		struct sqlresult * temp = result->result;
		if( result->total == 1 ){
		    ret = strncmp(temp->colname[2],"license",7);//col name for car number

		    if(0 == ret){
			strcpy(carnum,temp->data[2]);//car number	
			//释放结果集资源
                	free_result(result);
		        pthread_mutex_unlock(&whitelist_mtx);
			
		        return 0;
		    }
			
	         }else{

		 }

		/*
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
		*/
	}
	else
	{
		printf("sql exec failed\n");
		//释放结果集资源
                free_result(result);
		pthread_mutex_unlock(&whitelist_mtx);

		return -1;

	}//end of if( ret == 0......

	pthread_mutex_unlock(&whitelist_mtx);

	//释放结果集资源
         free_result(result);

	return -1;
}
