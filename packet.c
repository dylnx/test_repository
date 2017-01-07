#include "system.h"
#include "packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

int InitSocket()
{
    int sockfd;
    struct sockaddr_in addr;
    socklen_t addrlen;
    int num;
    char buf[6144];
    if ((sockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
    {
        printf("Create socket failed!");
        return -1;
    }
#if 0
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(iport);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd,(struct sockaddr*)(&addr),sizeof(struct sockaddr_in)) == -1)
    {
        CloseSocket(sockfd);
        sockfd = -1;
    }
#endif

    return sockfd;
}
int CloseSocket(int s)
{
    if (s != -1)
        close(s);
    return 0;
}
int SocketWrite(int s, char* buffer, DWORD size, char* ip, WORD port)
{
    struct sockaddr_in addr;
    if (s != -1)
    {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);
        return sendto(s,(const char*)buffer,size,0,(struct sockaddr*)&addr,sizeof(struct sockaddr_in));
    }
    return 0;
}
int SocketRead(int s, char* buffer, DWORD size, char* ip, WORD* port)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int num = recvfrom(s,buffer,10,0,(struct sockaddr*)&addr,&len);
    if (num < 0)
    {
        printf ("recv error!");
        return -1;
    }
    printf("Message(%s%)from client.\nIp(%s),port(%d)",buffer,inet_ntoa(addr.sin_addr),htons(addr.sin_port));
    return 0;
}
