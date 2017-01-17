#ifndef TCPUTIL_H_QVNKNSMP
#define TCPUTIL_H_QVNKNSMP

#define __LINUX__
#define NO_BLOCKED  0
#define BLOCKED     1


int ConnectNetwork();
int DisconnectNetwork();
int ConnectTcpServer(char ip[], int port);
int ConnectTcpServerNonBlock(char ip[], int port, int timeout);
int CreateTcpServer(int port);
int AcceptClient(int fd);
int DisconnectTcpServer(int fd);
int TcpSendData(int fd, unsigned char buffer[], int n, int ms);
int TcpRecvData(int fd, unsigned char buffer[], int n, int ms);
int ConnectTcpServerWithLocalIP(char ip[], int port, char localip[], int localport);
int CreateTcpServerWithLocalIP(char localip[], int port);
int CheckSocketFlag(int fd, int ms);
int SetSockBlock(int sfd, int block);
int GetPeerIp(int fd, char *ip, int *len);

#endif /* end of include guard: TCPUTIL_H_QVNKNSMP */
