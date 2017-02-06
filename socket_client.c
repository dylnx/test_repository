#include "common_sz.h"
#include "tcputil.h"
#include "socket_client.h"
#include "basic_func.h"
#include "whitelist.h"
#include "log.h"

#define TIME_OUT_TIME 10



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



