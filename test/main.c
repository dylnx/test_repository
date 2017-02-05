#include <stdio.h>
#include "logfile.h"
#include "common_sz.h"
#include "cond_thread.h"
#include "pass_record.h"
#include "tcputil.h"
#include "whitelist.h"

#define RECORD_NUM 5000
int client_recv_whitelist1();

char     server_ip[] = "192.168.1.211";
int      server_port = 1234;
struct CondThread *g_PassRecordLogWriteFileThread;
struct CondThread *g_PassRecordSendThread;

int main(int argc, char *argv[])
{
    // Init the whitelist 
       WhiteListDatabaseInit();

#if 1	
    // Init the pass record log file
    InitPassRecordLogFile();
    g_PassRecordLogWriteFileThread = CreateCondThread(PassRecordLogHandle);

    ThreadRun(g_PassRecordLogWriteFileThread);

    g_PassRecordSendThread = CreateCondThread(PassRecordSendHandle);

    ThreadRun(g_PassRecordSendThread);


	// begin test
	struct SPassRecordLog   plog;
	memset(&plog, 0x00, sizeof(plog));
	struct SPassRecordLog   *fakedata =
		(struct SPassRecordLog*)malloc(sizeof(struct SPassRecordLog)*RECORD_NUM);
	int   i=0;
	while(1)
	{
		for(i=0;i<RECORD_NUM;i++)
		{	
		//	printf("K %d\n", i);
			strcpy(plog.m_Tid, "2000748137E208B9");
			plog.m_Channel = 1;
			plog.m_Direction = i;
			plog.m_Flag = 0;
			strcpy(plog.m_Timestamp, "2017/01/22 15:34:44");
				// fill the TimeStamp
				unsigned long long nowtimestamp;
				struct timeval  nowtime;
				gettimeofday(&nowtime, NULL);
				nowtimestamp = ((unsigned long long)(nowtime.tv_sec))*1000000L
							+ (unsigned long long)(nowtime.tv_usec);
				printf("xxxx %llu\n", nowtimestamp);
				plog.m_Meta.m_TimeStamp[0] = (unsigned int)(nowtimestamp>>32);
				plog.m_Meta.m_TimeStamp[1] = (unsigned int)(nowtimestamp&0xFFFFFFFF);
				struct tm* today=NULL;
				time_t second = nowtimestamp/1000000;
				today = localtime(&second);
				if( NULL == today )
				{	
					printf("failed\n"); exit(-1); 
				}
				printf("%04d-%02d-%02d %02d:%02d:%02d\n", 
					today->tm_year+1900,
					today->tm_mon+1,
					today->tm_mday,
					today->tm_hour,
					today->tm_min,
					today->tm_sec);
			memcpy(&fakedata[i], &plog, sizeof(plog)); 
			InsertPassRecordLog2(&plog);
	//		sleep(1);

		}//end of for(i=0;.....
//		send_pass_record1(fakedata, RECORD_NUM, 1);
	//	sleep(10);

	}//end of while(1)

	return 0;
#endif
#if 0
	while(1)
	{
		client_recv_whitelist1();
		sleep(1);
	}
#endif
	return 0;
}


/* --------------------------------------------------------------------------*/
/**
 * @功能  接收白名单
 *
 * @返回值   -1 : 失败  0: 成功
 */
/* ----------------------------------------------------------------------------*/
int client_recv_whitelist1()
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

    extern int jointcompute_id;
	int   request_id  = jointcompute_id;
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
//		printf("recv wl item[%d] %s success\n", i, recv_buff+i*item_size);
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
	starttime = (unsigned long long)(timeval.tv_sec)*1000000 + (unsigned long long)(timeval.tv_usec);
	ret = RefreshWLDatabase(recv_buff, item_count);
	gettimeofday(&timeval, NULL);
	endtime = (unsigned long long)(timeval.tv_sec)*1000000 + (unsigned long long)(timeval.tv_usec);
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
