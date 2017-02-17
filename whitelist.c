#include <stdio.h>
#include "common_sz.h"
#include "dbutils.h"
#include "sqlite3.h"
#include "whitelist.h"
#define TIME_OUT_TIME 10

const char  g_wldb_file[] = "whitelist.db";

struct sDB * g_whitelist;

static pthread_mutex_t whitelist_mtx;

/* --------------------------------------------------------------------------*/
/**
 * @功能  接收白名单
 *
 * @返回值   -1 : 失败  0: 成功
 */
/* ----------------------------------------------------------------------------*/
int ClientRecvWhiteList()
{
	int         send_timeout = 5;
	int         recv_timeout = 10;
	int         ret;
	int         fd;
	char        *recv_buff;
	int          recv_buff_len;
	ret = ConnectTcpServerNonBlock(server_ip, server_port, 5000);
	if( ret<=0 )
	{
		printf("Connect to Server[%s:%d] Failed\n", server_ip, server_port);
		return -1;
	}
	fd = ret;

    	extern int dev_id;
	int   request_id  = dev_id;
	// request the whitelist
	char request_cmd[] = "2000000";
	sprintf(request_cmd+1, "%02d", request_id);

	ret = TcpSendData(fd, request_cmd, 7, send_timeout*1000);
	// loop to recevie the whitelist
	if( ret!=7 )
	{
		printf("send failure\n");
		goto error;
	}
	char      msg_head[1024];
	memset(msg_head, 0x00, 1024);
	ret = TcpRecvData(fd, msg_head, 4, recv_timeout*1000);
	if( ret!=4 )
	{
		printf("recv head failure\n");
		goto error;
	}
	int     item_count = atoi(msg_head);
	int     item_size = 4+TID_LEN+CAR_NUM_LEN;
	printf("WL ITEM %d\n", item_count);
	recv_buff = (char*)calloc(item_count*item_size,1);
	int  i =0;
	for(;i<item_count;i++)
	{
		ret = TcpRecvData(fd, recv_buff+i*item_size, item_size, recv_timeout*1000);
		if( ret!=item_size )
		{
			printf("recv wl item[%d] failure\n", i);
			break;
		}
        //	printf("recv wl item[%d] %s success\n", i, recv_buff+i*item_size);
	}
	if( i!=item_count )
	{
		printf("wl item not ready\n");
		free(recv_buff);
		goto error;
	}
	printf("wl item %d recevied\n", item_count);
	
	// save the whiltelist
	struct timeval  timeval;
	gettimeofday(&timeval, NULL);
	unsigned long long starttime,endtime = 0;
	starttime = (unsigned long long)timeval.tv_sec * 1000000L + (unsigned long long)timeval.tv_usec;

        //同步全部白名单
	ret = RefreshWLDatabase(recv_buff, item_count);

	gettimeofday(&timeval, NULL);
	endtime = (unsigned long long)timeval.tv_sec *1000000L + (unsigned long long)timeval.tv_usec;
	printf("Update WL Ret %d Time %d us\n", ret, endtime-starttime);
	
	free(recv_buff);
	char   response_cmd[] = "2001";
	sprintf(response_cmd+1, "%02d", request_id);
	ret = TcpSendData(fd, response_cmd, 4, 2000);
	if( ret!=4 )
	{
		printf("send reponse failed\n");
		goto error;
	}
	printf("send reponse success\n");
	DisconnectTcpServer(fd);
	return 0;
error:
	DisconnectTcpServer(fd);
	return -1;
}



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
	sql_cmd = (char*)malloc(200*40+30);
	sprintf(sql_cmd, "delete from whitelist");

	pthread_mutex_lock(&whitelist_mtx);
	ret = db_query(g_whitelist, sql_cmd);
	printf("Table Clear %d\n", ret);
	sql_prefix_len = sprintf(sql_cmd, "insert into whitelist values ");
	for(i=0;i<count;i++)
	{
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
