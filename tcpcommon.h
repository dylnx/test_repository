#ifndef TCPCOMMON_H_GPTH3EYZ
#define TCPCOMMON_H_GPTH3EYZ
#include "common.h"

// connection status
enum con_status{
	DISCONNECT = 0,
	CONNECTTING,
	CONN_TIMEOUT,
	CONNECTED,
	LISTENING
};
#define BLOCKED 1  
#define NO_BLOCKED 0

// Connection Infomation
typedef struct connection
{
	int fd;
	int status;
	int recv_timeout;
	u64 last_msg_timestamp; // last recvied message timestamp
	Mbyte msg; // the message send or recv
	// 下面的字段必须设置
	int header_size;// 头部大小
	int length_offset; // 长度字段距报文头的偏移
	int length_size;// 长度字段的大小
	// 下面的字段可选择设置
	void *data; // remote infomation self defined
	void *reserved; // message cache
} MConn;

// 设置接收报文时的超时
int set_recv_timeout(MConn *session, int timeout);


#endif /* end of include guard: TCPCOMMON_H_GPTH3EYZ */
