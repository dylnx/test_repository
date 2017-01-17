#include "tcputil.h"
#ifdef __LINUX__
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <pthread.h>
#include <fcntl.h>
#endif




/*
功能：
	设置socket 阻塞模式
参数：
	sfd: 
    block: NO_BLOCKED 非阻塞  BLOCKED 阻塞 
返回：
	0/成功、-1/失败
*/
int SetSockBlock(int sfd, int block)
{
	int flags, s;
	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror ("fcntl");
		return -1;
	}
	if(block == NO_BLOCKED)
		flags |= O_NONBLOCK;
	else if(block == BLOCKED)
		flags &= ~O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);
	if (s == -1)
	{
		perror ("fcntl");
		return -1;
	}
	return 0;
}

/*
功能：
	获取连接对端的IP地址
参数：
	sfd: 
	ip: 要保存的ip地址
	len: ip地址长度
返回：
	0/成功、-1/失败
*/
int GetPeerIp(int fd, char *ip, int *len)
{
    size_t addr_size = sizeof(struct sockaddr_in);                                                                                                                        
    struct sockaddr *sa = malloc(addr_size);                                                                                                                               
	if ( ip == NULL || len == NULL ) return -1;
                                                                                                                                                                           
    if (getpeername(fd, sa, &addr_size) == -1) {                                                                                                                           
        return "\0";                                                                                                                                                       
    }                                                                                                                                                                      
	char *ip_address = (char*)malloc(INET_ADDRSTRLEN);
    memset(ip_address, 0, INET_ADDRSTRLEN);                                                                                                                               
    if (NULL == inet_ntop(sa->sa_family,                                                                                                                                   
                &(((struct sockaddr_in *)sa)->sin_addr),
                ip_address, INET_ADDRSTRLEN)) {                                                                                                                           
		*len = 0;                                                                                                                               
		return -1;
    }                                                                                                                                                                      
    free(sa);                                                                                                                                                              
	*len = strlen(ip_address);
	strcpy(ip, ip_address);
    return 0;                                                                                                                                                     
}

/*
功能：
	连接网络
参数：
返回：
	0/失败、1/成功
*/
int ConnectNetwork()
{
#ifdef __WINDOWS_XP__
	struct WSAData      winSock;
	int					ret;
	
	//初始化Windows网络环境
	ret=WSAStartup(MAKEWORD(2, 2), &winSock);
	if ( ret<0 )
		return 0;
#endif
	return 1;
}

/*
功能：
	断开网络
参数：
返回：
	0/失败、1/成功
*/
int DisconnectNetwork()
{
#ifdef __WINDOWS_XP__
	WSACleanup();
#endif
	return 1;
}

/*
功能：
	连接tcp服务器
参数：
	ip：
	port：
返回：
	0</失败、>=0/成功socket
*/
int ConnectTcpServer(char ip[], int port)
{
	struct sockaddr_in	serverAddr;
	int                 ret;
	int					serverSocket;
	
	ret = strlen(ip);
	if ( (ret<7) || (ret>16) )
		return -1;
	if ( (port<0) || (port>65535) )
		return -1;
	
	//取服务器IP地址
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons((unsigned short)(port & 0x0000FFFF));
	serverAddr.sin_addr.s_addr=inet_addr(ip); 
	
	//创建TCP socket
	serverSocket=socket(AF_INET, SOCK_STREAM, 0);
	if ( serverSocket<0 )
	{
#ifdef __WINDOWS_XP__
		ret = WSAGetLastError();
#endif
#ifdef __LINUX__
		ret = errno;
#endif
		return -ret;
	}
	
	// 连接服务器
	ret=connect(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if ( ret<0 )
	{
#ifdef __WINDOWS_XP__
		ret = WSAGetLastError();
#endif
#ifdef __LINUX__
		ret = errno;
#endif
#ifdef __WINDOWS_XP__
		closesocket(serverSocket);
#endif
#ifdef __LINUX__
		close(serverSocket);
#endif
		serverSocket = -1;
		return -ret;
	}
	
	return serverSocket;
}

/*
功能：
	连接tcp服务器(非阻塞方式)
参数：
	ip：
	port：
    timeout: 连接超时 ms
返回：
	-1 失败、-2 超时 >0/成功socket
*/
int ConnectTcpServerNonBlock(char ip[], int port, int timeout)
{
	struct sockaddr_in	serverAddr;
	int                 ret;
	int					serverSocket;
	
	ret = strlen(ip);
	if ( (ret<7) || (ret>16) )
		return -1;
	if ( (port<0) || (port>65535) )
		return -1;
	if ( (timeout<0) )
		return -1;
	
	//取服务器IP地址
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons((unsigned short)(port & 0x0000FFFF));
	serverAddr.sin_addr.s_addr=inet_addr(ip); 
	
	//创建TCP socket
	serverSocket=socket(AF_INET, SOCK_STREAM, 0);
	if ( serverSocket<0 )
	{
#ifdef __WINDOWS_XP__
		ret = WSAGetLastError();
#endif
#ifdef __LINUX__
		ret = errno;
#endif
		return -ret;
	}
	// 设置为非阻塞
	ret = SetSockBlock(serverSocket, NO_BLOCKED);
	if ( ret!=0 )
	{
		printf("set the socket non-blocked failed, we will try to connect by block mode\n");
	}
	
	// 连接服务器
	ret=connect(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if ( ret!=0 )
	{
#ifdef __WINDOWS_XP__
		ret = WSAGetLastError();
#endif
#ifdef __LINUX__
		ret = errno;
#endif
		if ( ret!=EINPROGRESS )
		{
			printf("connect error\n");
			close(serverSocket);
			return -1;
		}
		else
		{
			struct timeval tm;
			socklen_t len;
			int error = -1;
			int res;

			tm.tv_sec = timeout/1000;
			tm.tv_usec = timeout%1000 * (1000) ;

			fd_set set, rset;
			FD_ZERO(&set);
			FD_ZERO(&rset);
			FD_SET(serverSocket,&set);
			FD_SET(serverSocket,&rset);
			res = select(serverSocket+1,&rset,&set,NULL,&tm);
			if(res < 0) //select 出错，关闭连接
			{
				close(serverSocket);
				return -1;
			}
			else if(res == 0)//超时
			{
				close(serverSocket);
				return -2;
			}
			else
			{
				if(FD_ISSET(serverSocket,&set)&&!FD_ISSET(serverSocket, &rset))
				{
					return serverSocket;
				}
				else
				{
					close(serverSocket);
					return -1;
				}
			}
		}
	}
	return serverSocket;
}

/*
功能：
	连接tcp服务器 使用特定的本地IP
参数：
	ip：
	port：
返回：
	0</失败、>=0/成功socket
*/
int ConnectTcpServerWithLocalIP(char ip[], int port, char localip[], int localport)
{
	struct sockaddr_in	serverAddr;
	int                 ret;
	int					serverSocket;
	
	ret = strlen(ip);
	if ( (ret<7) || (ret>16) )
		return -1;
	if ( (port<0) || (port>65535) )
		return -1;
	
	//取服务器IP地址
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons((unsigned short)(port & 0x0000FFFF));
	serverAddr.sin_addr.s_addr=inet_addr(ip); 
	
	//创建TCP socket
	serverSocket=socket(AF_INET, SOCK_STREAM, 0);
	if ( serverSocket<0 )
	{
#ifdef __WINDOWS_XP__
		ret = WSAGetLastError();
#endif
#ifdef __LINUX__
		ret = errno;
#endif
		return -ret;
	}
	// 绑定本地地址
	ret = BindLocalAddress(serverSocket, localip, localport);
	if ( ret<0 )
	{
#ifdef __WINDOWS_XP__
		ret = WSAGetLastError();
#endif
#ifdef __LINUX__
		ret = errno;
#endif
#ifdef __WINDOWS_XP__
		closesocket(serverSocket);
#endif
#ifdef __LINUX__
		close(serverSocket);
#endif
		serverSocket = -1;
		return -ret;
	}
	
	// 连接服务器
	ret=connect(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if ( ret<0 )
	{
#ifdef __WINDOWS_XP__
		ret = WSAGetLastError();
#endif
#ifdef __LINUX__
		ret = errno;
#endif
#ifdef __WINDOWS_XP__
		closesocket(serverSocket);
#endif
#ifdef __LINUX__
		close(serverSocket);
#endif
		serverSocket = -1;
		return -ret;
	}
	
	return serverSocket;
}

/*
功能：
	绑定本地IP-Port
参数：
	fd：socket描述符  ip port 要绑定的地址
返回：
	-1/失败、=0/成功
*/
int BindLocalAddress(int fd, char ip[], int port)
{
	struct sockaddr_in localAddr;
	int                ret;

	ret = strlen(ip);
	if ( (ret<7) || (ret>16) )
		return -1;
	
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons((unsigned short)(port));
	localAddr.sin_addr.s_addr = inet_addr(ip);

	ret = bind(fd, (struct sockaddr*)&localAddr, sizeof(localAddr));
	return ret;
}

/*
功能：
	断开服务器
参数：
	serverSocket：
返回：
	0</失败、>=0/成功
*/
int DisconnectTcpServer(int serverSocket)
{
	if ( serverSocket>=0 )
#ifdef __WINDOWS_XP__
		closesocket(serverSocket);
#endif
#ifdef __LINUX__
		close(serverSocket);
#endif
	return 1;
}

/*
功能：
	向tcp socket发送buffer中的n字节数据
参数：
	socket：
	buffer：
	n：
	ms：超时，单位：毫秒
返回：
	-1: 发送错误
        >0: 实际发送的字节数
*/
int TcpSendData(int socket, unsigned char buffer[], int n, int ms)
{
	int			flags=0;
	int			nleft;
	int			nbytes;
	int			m;
	unsigned char		*ptr;
	fd_set			wrset;
	struct timeval		timeout;

	if ( ms<1 )
		ms=1000;
	timeout.tv_sec = ms / 1000;	
	timeout.tv_usec = ms % 1000;

	ptr=buffer;
	// 剩余未发送的字节数
	nleft=n;
	while ( nleft>0 )
	{
		// 初始化socket发送
		FD_ZERO(&wrset);
		FD_SET((unsigned int)socket,  &wrset);
		// 等待所有发送socket中至少一个就绪
		m=select(socket+1, NULL, &wrset, NULL, &timeout);
		// 超时
		if ( m==0 )
		{
//			return -1;
			return (n - nleft);
		}
		// 收到信号：继续
#ifdef __WINDOWS_XP__
		else if ( (m<0) && (WSAGetLastError()==WSAEINTR) )
#endif
#ifdef __LINUX__
		else if ( (m<0) && (errno==EINTR) )
#endif
		{
			continue;
		}
		// 网络错误
		else if ( m<0 )	
		{
			return -1;
		}
		// 如果我们需要的发送socket就绪
		if ( FD_ISSET((unsigned int)socket, &wrset) )
		{
			// 发送数据
			nbytes=send(socket, (char *)ptr, nleft, flags); 
			// 发送错误
			if ( nbytes<0 )	
			{
				// 收到信号：继续
#ifdef __WINDOWS_XP__
				if ( WSAGetLastError()==WSAEINTR ) 
#endif
#ifdef __LINUX__
				if ( errno==EINTR ) 
#endif
				{
					nbytes=0;
				}
				// 网络错误
				else
				{
					return -1;
				}
			}
			// 对方关闭
			else if ( nbytes==0 )
			{
//				break;
				return -1;
			}

			nleft -= nbytes;
			ptr += nbytes;
		}
	}
	return n-nleft;
}

/*
功能：
	从tcp socket接收n字节到buffer中
参数：
	socket：
	buffer：
	n：
	ms：超时，单位：毫秒
返回
	-1: 网络错误
    >=0: 实际接收的字节数目
*/
int TcpRecvData(int socket, unsigned char buffer[], int n, int ms)
{
	int					flags=0; 
	int					nleft;
	int					nbytes;
	int					m;
	unsigned char		*ptr;
	fd_set				rdset;
	struct timeval		timeout;

	if ( ms<1 )
		ms=1000;
	timeout.tv_sec = ms / 1000;
	timeout.tv_usec = ms % 1000;

	ptr=buffer;
	// 剩余未接收的字节数
	nleft=n;
	while (  nleft>0 )
	{
		// 初始化socket接收
		FD_ZERO(&rdset);
		FD_SET((unsigned int)socket,  &rdset);
		// 等待所有接收socket中至少一个就绪
		m=select(socket+1, &rdset, NULL, NULL, &timeout);
		// 超时
		if ( m==0 )
		{
//			return -1;
			return (n - nleft);
		}
		// 信号：继续
#ifdef __WINDOWS_XP__
		else if ( (m<0) && (WSAGetLastError()==WSAEINTR) )
#endif
#ifdef __LINUX__
		else if ( (m<0) && (errno==EINTR) )
#endif
		{
			continue;
		}
		// 网络错误
		else if ( m<0 )
		{
			return -1;
//			break;
		}

		// 如果我们需要的接收socket就绪
		if ( FD_ISSET((unsigned int)socket, &rdset) )
		{
			//接收
			nbytes=recv(socket, (char *)ptr, nleft, flags); 
			if ( nbytes<0 )
			{
				// 信号：继续
#ifdef __WINDOWS_XP__
				if ( WSAGetLastError()==WSAEINTR ) 
#endif
#ifdef __LINUX__
				if ( errno==EINTR ) 
#endif
				{
					nbytes=0;
				}
				// 网络错误
				else
				{
					return -1;
				}
			}
			// 对方关闭
			else if ( nbytes==0 ) 
			{
//				break;
				return -1;
			}

			nleft -= nbytes;
			ptr += nbytes;
		}
	}

	return n-nleft;
}


/* 
 * 0: 正常
 * 1: 超时
 * 2: 读信号
 * 3: 写信号
 * 4: 异常信号
 * -1: 连接断开
 */
int CheckSocketFlag(int fd, int ms)
{
	int m;
	fd_set				rdset,wdset,edset;
	struct timeval		timeout;
	if ( ms<1 )
		ms=1000;
	timeout.tv_sec = ms / 1000;
	timeout.tv_usec = ms % 1000;

	// 初始化socket接收
	FD_ZERO(&rdset);
//	FD_ZERO(&wdset);
	FD_ZERO(&edset);
	FD_SET((unsigned int)fd,  &rdset);
//	FD_SET((unsigned int)fd,  &wdset);
	FD_SET((unsigned int)fd,  &edset);

	// 等待所有接收socket中至少一个就绪
	m=select(fd+1, &rdset, NULL, &edset, &timeout);
	// 超时
	if ( m==0 )
	{
		return 1;
	}
	else if ( m<0  && EINTR == errno )
	{
		return 0;
	}
	else if ( m < 0 )
	{
		return -1;
	}
	else
	{
		if ( FD_ISSET((unsigned int)fd, &rdset) )
		{
	//		printf("Read\n");
			return 2;
		}
		if ( FD_ISSET((unsigned int)fd, &edset) )
		{
			return 4;
		}
		return -1;
	}
}

int CreateTcpServer(int port)
{
	if ( (port<0) || (port>65535) )
		return -1;
    int s, sfd;

	struct sockaddr_in       my_addr;
	memset(&my_addr, 0x00, sizeof(my_addr));
    my_addr.sin_family = AF_INET;     
    my_addr.sin_port = htons(port); 
	my_addr.sin_addr.s_addr = INADDR_ANY;

    sfd = socket (AF_INET, SOCK_STREAM, 0);
	if ( sfd < 0 ){
		printf("create socket error\n");
		return -1;
	}
	int n =1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));
    s = bind (sfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
    if (s < 0 )
    {   
        close (sfd);
		return -1;
    }   
	
    if (listen(sfd, 1) == -1) {
        close(sfd);
        printf("listen(): %s", strerror(errno));
        return -1;
    }
	return sfd;
}

int CreateTcpServerWithLocalIP(char localip[], int port)
{
	int ret;
	ret = strlen(localip);
	if ( (ret<7) || (ret>16) )
		return -1;
	if ( (port<0) || (port>65535) )
		return -1;
    int s, sfd;

	struct sockaddr_in       my_addr;
	memset(&my_addr, 0x00, sizeof(my_addr));
    my_addr.sin_family = AF_INET;     
    my_addr.sin_port = htons(port); 
	my_addr.sin_addr.s_addr = inet_addr(localip);

    sfd = socket (AF_INET, SOCK_STREAM, 0);
	if ( sfd < 0 ){
		printf("create socket error\n");
		return -1;
	}
	int n =1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));
    s = bind (sfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
    if (s < 0 )
    {   
        close (sfd);
		return -1;
    }   
	
    if (listen(sfd, 1) == -1) {
        close(sfd);
        printf("listen(): %s", strerror(errno));
        return -1;
    }
	return sfd;
}
int AcceptClient(int fd)
{
	if( fd <= 0) return -1;
	struct sockaddr_in     client_addr;
	int                    client_fd;
	int                    sin_size ;
	sin_size = sizeof(struct sockaddr_in);
	client_fd = accept(fd, (struct sockaddr*)&client_addr, &sin_size);
	if( client_fd == -1 )
	{
		printf("accept error\n");
		return -1;
	}
	return client_fd;
}
