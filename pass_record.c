#include "pass_record.h"

/*
功能：将通行记录发送到后台服务端，并标记发送成功或失败状态
参数：sr 通行记录指针
      flag 旧数据标志  
返回值：详见PK_STATUS定义
*/
PK_STATUS send_pass_record(struct SPassRecordLog *sr, int flag)
{
        struct SPassRecordLog m_sr;
	//参数校验
	if(NULL == sr){
		return PK_BAD_PARAMETER_1;
	}
	memset(&m_sr,0,sizeof(m_sr));	
	memcpy((struct SPassRecordLog *)&m_sr,sr,sizeof(struct SPassRecordLog));

	//尝试与服务器建立连接
	int sockfd = ConnectTcpServerNonBlock(server_ip,server_port,5000);
	if( -1 == sockfd ){
		return PK_CONNECT_FAILED;
	}else if( -2 == sockfd ){
		return PK_CONNECT_TIMEOUT;
	}
	//printf("connected  server successfully!!!\n");
	
	//申请临时缓冲区
	unsigned char *m_sbuff = (unsigned char*)calloc(1,4096);
	unsigned char m_len;
	if( NULL == m_sbuff ){
		DisconnectTcpServer(sockfd);	
		return PK_LOW_MEMORY;
	}

	//组织要发送的记录字符串
	m_len = sprintf(m_sbuff, "%1d%02d%04d", 1, 0, 1);//类型+通道号+记录条数 
	m_len += sprintf(m_sbuff+m_len, "$%02d%s%1d%s", m_sr.m_Channel, 
			m_sr.m_Tid, 
			m_sr.m_PassResult, 
			m_sr.m_Timestamp);


        
	//m_sr.m_Flag = 1;//标记发送失败

	//发送数据
	int m_sret = 0;
	m_sret = TcpSendData(sockfd,m_sbuff,m_len,10);
	if( m_len!=m_sret ){
		DisconnectTcpServer(sockfd);	
		free(m_sbuff);
		return  PK_SEND_FAILED;
	}
	print_log(f_sended_server,"send%dbytes\n", m_len);
	free(m_sbuff);

	//接收server应签
	unsigned char m_rbuff[6] = {0};
	int m_rret = 0;	
	m_rret = TcpRecvData(sockfd,m_rbuff,4,10000);	
	if( 4 !=m_rret )
	{
		print_log(f_sended_server,"ERROR:recv data failed,recv count less 4 Bytes!!!\n");
		DisconnectTcpServer(sockfd);	
		return PK_RECV_FAILED;
	}
	if( m_rbuff[3]=='0')
	{
		print_log(f_sended_server,"ERROR: response code 0!!!\n");
		DisconnectTcpServer(sockfd);	
		return PK_RECV_FAILED;
	}
	m_rbuff[4] = 0;
	DisconnectTcpServer(sockfd);	
	print_log(f_sended_server,"OK:send data successfully!!!\n");
	return PK_SUCCESS;
}


/*
功能：将通行记录发送到后台服务端，multi 并标记发送成功或失败状态
参数：sr 通行记录指针
      flag 旧数据标志  1
返回值：详见PK_STATUS定义
*/
PK_STATUS send_pass_record1(struct SPassRecordLog *sr, int count, int flag)
{
        struct SPassRecordLog m_sr;
	//参数校验
	if(NULL == sr){
		return PK_BAD_PARAMETER_1;
	}
	if( count > 9999 ) 
	{
		print_log(f_sended_server,"ERROR:count too large!!!\n");
		return PK_BAD_PARAMETER_2;
	}
	memset(&m_sr,0,sizeof(m_sr));	

	//尝试与服务器建立连接
	int sockfd = ConnectTcpServerNonBlock(server_ip,server_port,5000);
	if( -1 == sockfd ){
		return PK_CONNECT_FAILED;
	}else if( -2 == sockfd ){
		return PK_CONNECT_TIMEOUT;
	}
	//printf("connected  server successfully!!!\n");
	
	//申请临时缓冲区
	unsigned char *m_sbuff = (unsigned char*)calloc(1,4096);
	unsigned char m_len;
	if( NULL == m_sbuff ){
		DisconnectTcpServer(sockfd);	
		return PK_LOW_MEMORY;
	}

	//组织要发送的报文头
	m_len = sprintf(m_sbuff, "%1d%02d%04d", 1, 0, count);//类型+通道号+记录条数 
	//发送报文头
	int m_sret = 0;
	m_sret = TcpSendData(sockfd,m_sbuff,m_len,10000);
	if( m_len!=m_sret ){
		DisconnectTcpServer(sockfd);	
		print_log(f_sended_server,"ERROR:send head data failed!!!\n");
		return PK_SEND_FAILED;
	}
	int send_count=0;
	while(send_count<count)
	{
		memcpy(&m_sr, &sr[send_count], sizeof(m_sr));
		m_len = sprintf(m_sbuff, "$%02d%s%1d%s", m_sr.m_Channel, 
			m_sr.m_Tid, 
			m_sr.m_PassResult, 
			m_sr.m_Timestamp);
		//printf("[%d] %d %s\n", send_count ,m_len , m_sbuff);
		m_sret = TcpSendData(sockfd,m_sbuff,m_len,10000);
		if( m_len!=m_sret ){
			DisconnectTcpServer(sockfd);	
			print_log(f_sended_server,"ERROR:send %d'th data failed!!!\n", send_count);
			return PK_SEND_FAILED;
		}	
		send_count++;
		print_log(f_sended_server,"[%d] send success\n", send_count);
	}
	
	free(m_sbuff);
	//接收server应签
	unsigned char m_rbuff[6] = {0};
	int m_rret = 0;	
	m_rret = TcpRecvData(sockfd,m_rbuff,4,10000);	
	if( 4 !=m_rret )
	{
		print_log(f_sended_server,"ERROR:recv data failed,recv count less 4 Bytes!!!\n");
		DisconnectTcpServer(sockfd);	
		return PK_RECV_FAILED;
	}
	if( m_rbuff[3]=='0')
	{
		print_log(f_sended_server,"ERROR: response code 0!!!\n");
		DisconnectTcpServer(sockfd);	
		return PK_RECV_FAILED;
	}
	m_rbuff[4] = 0;
	print_log(f_sended_server,"response : %s\n", m_rbuff); 
	DisconnectTcpServer(sockfd);	
	print_log(f_sended_server,"OK:send data successfully!!!\n");
	return PK_SUCCESS;
}
