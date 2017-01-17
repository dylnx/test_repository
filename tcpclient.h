#ifndef TCPCLIENT_H_XJERFOYS
#define TCPCLIENT_H_XJERFOYS

#include "common.h"
#include "tcpcommon.h"

#define CONNECT_TIMEOUT 500 //ms 500ms is default value
#define RECV_TIMEOUT 10000 //ms 10s is default value

// 创建主动连接
MConn create_connect(const char* ip, int port, int timeout);

// 关闭连接
int close_connect(MConn conn);

#endif /* end of include guard: TCPCLIENT_H_XJERFOYS */
