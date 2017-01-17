#include "pass_record.h"
/*
功能：将通行记录发送到后台服务端，并标记发送成功或失败状态
参数：通行记录指针
返回值：
*/
int send_pass_record(struct SPassRecordLog *sr)
{
        struct SPassRecordLog m_sr;
	//参数校验
	if(NULL == sr){
		print_log(f_sended_server,"ERROR:parameter invilided!!!\n");
		return -1;
	}
	memset(&m_sr,0,sizeof(m_sr));	
	memcpy((struct SPassRecordLog *)&m_sr,sr,sizeof(struct SPassRecordLog));

	//尝试与服务器建立连接
	int sockfd = ConnectTcpServerNonBlock(server_ip,server_port,5000);
	if( sockfd >= 0 ){
		print_log(f_sended_server,"ERROR:connected  server successfully!!!\n");
		return sockfd;
	}else if( -1 == sockfd ){
		print_log(f_sended_server,"ERROR:cann't connect server!!!\n");
		return -1;
	}else if( -2 == sockfd ){
		print_log(f_sended_server,"ERROR:connect server timeout!!!\n");
		return -1;
	}
	
	//申请临时缓冲区
	unsigned char m_sbuff = (unsigned char*)calloc(1,4096);
	unsigned char m_len;
	if( NULL == m_sbuff ){
		print_log(f_sended_server,"ERROR:calloc m_sbuff is NULL!!!\n");
		return -1;
	}

	//组织要发送的记录字符串
	m_len = sprintf(m_sbuff, "%1d%02d%04d", 1, 0, 1);//类型+通道号+记录条数 
	m_len += sprintf(m_sbuff+m_len, "$%02d%s%1d%s", m_sr.m_Channel, \
	m_sr.m_Tid, m_sr.m_PassResult, m_sr.m_Timestamp);

	printf("%d %s\n", m_len , m_sbuff);
	

        
	m_sr.m_Flag = 1;//标记发送失败
	//发送数据
	int m_sret = 0;
	m_sret = TcpSendData(sockfd,m_sbuff,m_len,10);
	if( m_len!=m_sret ){
		InsertPassPassRecordLog1(&m_sr);
		DisconnectTcpServer(sockfd);	
		free(m_sbuff);

		print_log(f_sended_server,"ERROR:send data failed!!!\n");

		return -1;
	}

	//接收server应签
	unsigned char m_rbuff[6] = {0};
	int m_rret = 0;	
	m_rret = TcpRecvData(sockfd,m_rbuff,4,10000);	
	if( 4 !=m_rret )
	{
		InsertPassPassRecordLog1(&m_sr);
		DisconnectTcpServer(sockfd);	
		free(m_sbuff);
		print_log(f_sended_server,"ERROR:recv data failed,recv count less 4 Bytes!!!\n");
		return -1;
	}
	if( m_rbuff[3]=='0')
	{
		InsertPassPassRecordLog1(&m_sr);
		DisconnectTcpServer(sockfd);	
		free(m_sbuff);
		print_log(f_sended_server,"ERROR:server reponse no ok!!!\n");
		return -1;
	}
	m_sr.m_Flag =0;
	InsertPassPassRecordLog1(&m_sr);
	DisconnectTcpServer(sockfd);	
	free(m_sbuff);
	print_log(f_sended_server,"OK:send data successfully!!!\n");
	return 0;

}
