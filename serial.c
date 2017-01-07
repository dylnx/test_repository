//\u4e32\u53e3\u76f8\u5173\u7684\u5934\u6587\u4ef6
#include<stdio.h>      /*\u6807\u51c6\u8f93\u5165\u8f93\u51fa\u5b9a\u4e49*/
#include<stdlib.h>     /*\u6807\u51c6\u51fd\u6570\u5e93\u5b9a\u4e49*/
#include<unistd.h>     /*Unix \u6807\u51c6\u51fd\u6570\u5b9a\u4e49*/
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>      /*\u6587\u4ef6\u63a7\u5236\u5b9a\u4e49*/
#include<termios.h>    /*PPSIX \u7ec8\u7aef\u63a7\u5236\u5b9a\u4e49*/
#include<errno.h>      /*\u9519\u8bef\u53f7\u5b9a\u4e49*/
#include<string.h>
#include "serial.h"

#define COM1 "/dev/ttymxc1"
#define COM2 "/dev/ttymxc2"
#define COM3 "/dev/ttymxc3"
#define COM4 "/dev/ttymxc4"
#define COM5 "/dev/ttyHBB0"
#define COM6 "/dev/ttyHBB1"
#define COM7 "/dev/ttyHBB2"
#define COM8 "/dev/ttyHBB3"
#define COM9 "/dev/ttyUSB0"
#define command_1 {0x01, 0x06, 0x00, 0x01, 0x01, 0x00, 0xd9, 0x9a}
#define command_2 {0x01, 0x06, 0x00, 0x01, 0x01, 0x01, 0x18, 0x5a}

#define command_3 {0x01, 0x06, 0x00, 0x01, 0x02, 0x00, 0xd9, 0x6a}
#define command_4 {0x01, 0x06, 0x00, 0x01, 0x02, 0x01, 0x18, 0xaa}

#define command_5 {0x01, 0x06, 0x00, 0x01, 0x03, 0x00, 0xd8, 0xfa}
#define command_6 {0x01, 0x06, 0x00, 0x01, 0x03, 0x01, 0x19, 0x3a}

#define command_7 {0x01, 0x06, 0x00, 0x01, 0x04, 0x00, 0xda, 0xca}
#define command_8 {0x01, 0x06, 0x00, 0x01, 0x04, 0x01, 0x1b, 0x0a}

#define command_9 {0x01, 0x06, 0x00, 0x01, 0x00, 0x00, 0xd8, 0x0a}
#define command_10 {0x01, 0x06, 0x00, 0x01, 0xff, 0xff, 0xd9, 0xba}

int com_port = 1;

char com_devices[9][13] = { COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9};


const char open_command[8] = command_2;
const char stop_command[8] = command_1;
//\u5b8f\u5b9a\u4e49
#define FALSE  -1
#define TRUE   0

/*******************************************************************
* \u540d\u79f0\uff1a                  UART0_Open
* \u529f\u80fd\uff1a                \u6253\u5f00\u4e32\u53e3\u5e76\u8fd4\u56de\u4e32\u53e3\u8bbe\u5907\u6587\u4ef6\u63cf\u8ff0
* \u5165\u53e3\u53c2\u6570\uff1a        fd    :\u6587\u4ef6\u63cf\u8ff0\u7b26     port :\u4e32\u53e3\u53f7(ttyS0,ttyS1,ttyS2)
* \u51fa\u53e3\u53c2\u6570\uff1a        \u6b63\u786e\u8fd4\u56de\u4e3a1\uff0c\u9519\u8bef\u8fd4\u56de\u4e3a0
*******************************************************************/
int UART0_Open(int fd,char* port)
{

         fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
         if (FALSE == fd)
                {
                       perror("Can't Open Serial Porttt");
                       return(FALSE);
                }
   /*  //\u6062\u590d\u4e32\u53e3\u4e3a\u963b\u585e\u72b6\u6001
     if(fcntl(fd, F_SETFL, 0) < 0)
                {
                       printf("fcntl failed!\n");
                     return(FALSE);
                }
         else
                {
                  printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
                }
		 */
/*
      //\u6d4b\u8bd5\u662f\u5426\u4e3a\u7ec8\u7aef\u8bbe\u5907
      if(0 == isatty(STDIN_FILENO))
                {
                       printf("standard input is not a terminal device\n");
                  return(FALSE);
                }
  else
                {
                     printf("isatty success!\n");
                }
				*/
  return fd;
}
/*******************************************************************
* \u540d\u79f0\uff1a                UART0_Close
* \u529f\u80fd\uff1a                \u5173\u95ed\u4e32\u53e3\u5e76\u8fd4\u56de\u4e32\u53e3\u8bbe\u5907\u6587\u4ef6\u63cf\u8ff0
* \u5165\u53e3\u53c2\u6570\uff1a        fd    :\u6587\u4ef6\u63cf\u8ff0\u7b26     port :\u4e32\u53e3\u53f7(ttyS0,ttyS1,ttyS2)
* \u51fa\u53e3\u53c2\u6570\uff1a        void
*******************************************************************/

void UART0_Close(int fd)
{
    close(fd);
}

/*******************************************************************
* \u540d\u79f0\uff1a                UART0_Set
* \u529f\u80fd\uff1a                \u8bbe\u7f6e\u4e32\u53e3\u6570\u636e\u4f4d\uff0c\u505c\u6b62\u4f4d\u548c\u6548\u9a8c\u4f4d
* \u5165\u53e3\u53c2\u6570\uff1a        fd        \u4e32\u53e3\u6587\u4ef6\u63cf\u8ff0\u7b26
*                              speed     \u4e32\u53e3\u901f\u5ea6
*                              flow_ctrl   \u6570\u636e\u6d41\u63a7\u5236
*                           databits   \u6570\u636e\u4f4d   \u53d6\u503c\u4e3a 7 \u6216\u80058
*                           stopbits   \u505c\u6b62\u4f4d   \u53d6\u503c\u4e3a 1 \u6216\u80052
*                           parity     \u6548\u9a8c\u7c7b\u578b \u53d6\u503c\u4e3aN,E,O,,S
*\u51fa\u53e3\u53c2\u6570\uff1a          \u6b63\u786e\u8fd4\u56de\u4e3a1\uff0c\u9519\u8bef\u8fd4\u56de\u4e3a0
*******************************************************************/
int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{

      int   i;
         int   status;
         int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
     int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};

    struct termios options;

    /*tcgetattr(fd,&options)\u5f97\u5230\u4e0efd\u6307\u5411\u5bf9\u8c61\u7684\u76f8\u5173\u53c2\u6570\uff0c\u5e76\u5c06\u5b83\u4eec\u4fdd\u5b58\u4e8eoptions,\u8be5\u51fd\u6570\u8fd8\u53ef\u4ee5\u6d4b\u8bd5\u914d\u7f6e\u662f\u5426\u6b63\u786e\uff0c\u8be5\u4e32\u53e3\u662f\u5426\u53ef\u7528\u7b49\u3002\u82e5\u8c03\u7528\u6210\u529f\uff0c\u51fd\u6570\u8fd4\u56de\u503c\u4e3a0\uff0c\u82e5\u8c03\u7528\u5931\u8d25\uff0c\u51fd\u6570\u8fd4\u56de\u503c\u4e3a1.
    */
    if  ( tcgetattr( fd,&options)  !=  0)
       {
          perror("SetupSerial 1");
          return(FALSE);
       }

    //\u8bbe\u7f6e\u4e32\u53e3\u8f93\u5165\u6ce2\u7279\u7387\u548c\u8f93\u51fa\u6ce2\u7279\u7387
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
                {
                     if  (speed == name_arr[i])
                            {
                                 cfsetispeed(&options, speed_arr[i]);
                                 cfsetospeed(&options, speed_arr[i]);
                            }
              }

    //\u4fee\u6539\u63a7\u5236\u6a21\u5f0f\uff0c\u4fdd\u8bc1\u7a0b\u5e8f\u4e0d\u4f1a\u5360\u7528\u4e32\u53e3
    options.c_cflag |= CLOCAL;
    //\u4fee\u6539\u63a7\u5236\u6a21\u5f0f\uff0c\u4f7f\u5f97\u80fd\u591f\u4ece\u4e32\u53e3\u4e2d\u8bfb\u53d6\u8f93\u5165\u6570\u636e
    options.c_cflag |= CREAD;

    //\u8bbe\u7f6e\u6570\u636e\u6d41\u63a7\u5236
    switch(flow_ctrl)
    {

       case 0 ://\u4e0d\u4f7f\u7528\u6d41\u63a7\u5236
              options.c_cflag &= ~CRTSCTS;
              break;

       case 1 ://\u4f7f\u7528\u786c\u4ef6\u6d41\u63a7\u5236
              options.c_cflag |= CRTSCTS;
              break;
       case 2 ://\u4f7f\u7528\u8f6f\u4ef6\u6d41\u63a7\u5236
              options.c_cflag |= IXON | IXOFF | IXANY;
              break;
    }
    //\u8bbe\u7f6e\u6570\u636e\u4f4d
    //\u5c4f\u853d\u5176\u4ed6\u6807\u5fd7\u4f4d
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
       case 5    :
                     options.c_cflag |= CS5;
                     break;
       case 6    :
                     options.c_cflag |= CS6;
                     break;
       case 7    :
                 options.c_cflag |= CS7;
                 break;
       case 8:
                 options.c_cflag |= CS8;
                 break;
       default:
                 fprintf(stderr,"Unsupported data size\n");
                 return (FALSE);
    }
    //\u8bbe\u7f6e\u6821\u9a8c\u4f4d
    switch (parity)
    {
       case 'n':
       case 'N': //\u65e0\u5947\u5076\u6821\u9a8c\u4f4d\u3002
                 options.c_cflag &= ~PARENB;
                 options.c_iflag &= ~INPCK;
                 break;
       case 'o':
       case 'O'://\u8bbe\u7f6e\u4e3a\u5947\u6821\u9a8c
                 options.c_cflag |= (PARODD | PARENB);
                 options.c_iflag |= INPCK;
                 break;
       case 'e':
       case 'E'://\u8bbe\u7f6e\u4e3a\u5076\u6821\u9a8c
                 options.c_cflag |= PARENB;
                 options.c_cflag &= ~PARODD;
                 options.c_iflag |= INPCK;
                 break;
       case 's':
       case 'S': //\u8bbe\u7f6e\u4e3a\u7a7a\u683c
                 options.c_cflag &= ~PARENB;
                 options.c_cflag &= ~CSTOPB;
                 break;
        default:
                 fprintf(stderr,"Unsupported parity\n");
                 return (FALSE);
    }
    // \u8bbe\u7f6e\u505c\u6b62\u4f4d
    switch (stopbits)
    {
       case 1:
                 options.c_cflag &= ~CSTOPB; break;
       case 2:
                 options.c_cflag |= CSTOPB; break;
       default:
                       fprintf(stderr,"Unsupported stop bits\n");
                       return (FALSE);
    }

  //\u4fee\u6539\u8f93\u51fa\u6a21\u5f0f\uff0c\u539f\u59cb\u6570\u636e\u8f93\u51fa
  options.c_oflag &= ~OPOST;

  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//\u6211\u52a0\u7684
//options.c_lflag &= ~(ISIG | ICANON);

    //\u8bbe\u7f6e\u7b49\u5f85\u65f6\u95f4\u548c\u6700\u5c0f\u63a5\u6536\u5b57\u7b26
    options.c_cc[VTIME] = 1; /* \u8bfb\u53d6\u4e00\u4e2a\u5b57\u7b26\u7b49\u5f851*(1/10)s */
    options.c_cc[VMIN] = 1; /* \u8bfb\u53d6\u5b57\u7b26\u7684\u6700\u5c11\u4e2a\u6570\u4e3a1 */

    //\u5982\u679c\u53d1\u751f\u6570\u636e\u6ea2\u51fa\uff0c\u63a5\u6536\u6570\u636e\uff0c\u4f46\u662f\u4e0d\u518d\u8bfb\u53d6 \u5237\u65b0\u6536\u5230\u7684\u6570\u636e\u4f46\u662f\u4e0d\u8bfb
    tcflush(fd,TCIFLUSH);

    //\u6fc0\u6d3b\u914d\u7f6e (\u5c06\u4fee\u6539\u540e\u7684termios\u6570\u636e\u8bbe\u7f6e\u5230\u4e32\u53e3\u4e2d\uff09
    if (tcsetattr(fd,TCSANOW,&options) != 0)
           {
               perror("com set error!\n");
              return (FALSE);
           }
    return (TRUE);
}
/*******************************************************************
* \u540d\u79f0\uff1a                UART0_Init()
* \u529f\u80fd\uff1a                \u4e32\u53e3\u521d\u59cb\u5316
* \u5165\u53e3\u53c2\u6570\uff1a        fd       :  \u6587\u4ef6\u63cf\u8ff0\u7b26
*               speed  :  \u4e32\u53e3\u901f\u5ea6
*                              flow_ctrl  \u6570\u636e\u6d41\u63a7\u5236
*               databits   \u6570\u636e\u4f4d   \u53d6\u503c\u4e3a 7 \u6216\u80058
*                           stopbits   \u505c\u6b62\u4f4d   \u53d6\u503c\u4e3a 1 \u6216\u80052
*                           parity     \u6548\u9a8c\u7c7b\u578b \u53d6\u503c\u4e3aN,E,O,,S
*
* \u51fa\u53e3\u53c2\u6570\uff1a        \u6b63\u786e\u8fd4\u56de\u4e3a1\uff0c\u9519\u8bef\u8fd4\u56de\u4e3a0
*******************************************************************/
int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    int err;
    //\u8bbe\u7f6e\u4e32\u53e3\u6570\u636e\u5e27\u683c\u5f0f
    if (UART0_Set(fd,speed,flow_ctrl,databits,stopbits,parity) == FALSE)
       {
        return FALSE;
       }
    else
       {
               return  TRUE;
        }
}

/*******************************************************************
* \u540d\u79f0\uff1a                  UART0_Recv
* \u529f\u80fd\uff1a                \u63a5\u6536\u4e32\u53e3\u6570\u636e
* \u5165\u53e3\u53c2\u6570\uff1a        fd                  :\u6587\u4ef6\u63cf\u8ff0\u7b26
*                              rcv_buf     :\u63a5\u6536\u4e32\u53e3\u4e2d\u6570\u636e\u5b58\u5165rcv_buf\u7f13\u51b2\u533a\u4e2d
*                              data_len    :\u4e00\u5e27\u6570\u636e\u7684\u957f\u5ea6
* \u51fa\u53e3\u53c2\u6570\uff1a        \u6b63\u786e\u8fd4\u56de\u4e3a1\uff0c\u9519\u8bef\u8fd4\u56de\u4e3a0
*******************************************************************/
int UART0_Recv(int fd, char *rcv_buf,int data_len)
{
    int len,fs_sel;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    time.tv_sec = 10;
    time.tv_usec = 0;

    //\u4f7f\u7528select\u5b9e\u73b0\u4e32\u53e3\u7684\u591a\u8def\u901a\u4fe1
    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
    if(fs_sel)
       {
              len = read(fd,rcv_buf,data_len);
	      printf("I am right!(version1.2) len = %d fs_sel = %d\n",len,fs_sel);
              return len;
       }
    else
       {
	      printf("Sorry,I am wrong!");
              return FALSE;
       }
}
/********************************************************************
* \u540d\u79f0\uff1a                  UART0_Send
* \u529f\u80fd\uff1a                \u53d1\u9001\u6570\u636e
* \u5165\u53e3\u53c2\u6570\uff1a        fd                  :\u6587\u4ef6\u63cf\u8ff0\u7b26
*                              send_buf    :\u5b58\u653e\u4e32\u53e3\u53d1\u9001\u6570\u636e
*                              data_len    :\u4e00\u5e27\u6570\u636e\u7684\u4e2a\u6570
* \u51fa\u53e3\u53c2\u6570\uff1a        \u6b63\u786e\u8fd4\u56de\u4e3a1\uff0c\u9519\u8bef\u8fd4\u56de\u4e3a0
*******************************************************************/
int UART0_Send(int fd, char *send_buf,int data_len)
{
    int len = 0;

    len = write(fd,send_buf,data_len);
    if (len == data_len )
              {
                     return len;
              }
    else
        {

                tcflush(fd,TCOFLUSH);
                return FALSE;
        }
}

int com_init(char * com_name)
{
    int fd = UART0_Open(fd,com_name);
    int err = UART0_Init(fd,9600,0,8,1,'N');
    if (-1 == err || 0 >= fd){
        printf("Set Port Error!!!!!\n");
        UART0_Close(fd);
        return 0;
    }
    return fd;
}

int com_open_roadblock(int fd,int gateNum)
{                        
    int err;                           
    int len;
    char rcv_buf[100]={0};
    char send_buf[100] = {0};
    if(NULL ==fd)
	return -1;
    if(0 !=gateNum && 1 !=gateNum)
	return -1;

        send_buf[0]=0xaa;
    if(0 == gateNum)
    {
        send_buf[1]=0x02;
	send_buf[2]=0x02;
    }else if(1==gateNum){
        send_buf[1] = 0x02;
        send_buf[2] = 0x02;
    }
        send_buf[3]=0x33;
        send_buf[4]=0x33;

    len = UART0_Send(fd,send_buf,5);
    if(len == 5)
    {
        return 0;
    }
    else
    {
        printf("send data failed!\n");
	return -1; 
    }
    return 0;
}

int Ctrl_gateway(int fd,char* cmd)
{
    int len;
    char send_buf[100];
    if(-1 == fd || 0>=fd) 
        return -1;
    if(NULL == cmd)
        return -1;
    //unsigned char send_buf[] = {0xaa,0x01,0x01,0x33,0x33};

    len = UART0_Send(fd,send_buf,5);
    if(len == strlen(send_buf))
    {
    }
    else
    {
	return -1;
        printf("send data failed!\n");
    }
    return 0;
	
	
}

int GetGatewayStat(int fd,int gateNum)
{
    int len;
    //char rcv_buf[100];
    char send_buf[30]={0};
    char recv_buf[30]={0};
    if(-1 == fd || 0>=fd) 
        return -1;

    if(0 !=gateNum || 1 !=gateNum)
	return -1;

        send_buf[0]=0xaa;
        send_buf[3]=0x30;
        send_buf[4]=0x30;
    if(0 == gateNum)
    {
        send_buf[1]=0x01;
	send_buf[2]=0x01;
    }else{
        send_buf[1] = 0x02;
        send_buf[2] = 0x02;
    }




    len = UART0_Send(fd,send_buf,5);
    if(len == strlen(send_buf))
    {
    }
    else
    {
	return -1;
        printf("send data failed!\n");
    }
    usleep(300*100);
    len = UART0_Recv(fd,recv_buf,7); //the stat code is 7 Byte
    if(len != 7)
    {
	return -1;
    }else{
	
    	if(0 == gateNum)
    	{
           if(	      (recv_buf[0]==0xcc) && \
              	      (recv_buf[1]==0x01) && \
	      	      (recv_buf[2]==0x01) && \
              	      (recv_buf[3]==0x30) && \
              	      (recv_buf[4]==0x30) && \
              	      (recv_buf[5]==0x30) && \
              	      (recv_buf[6]==0x30)){
		  return 7;//define open satus 7
             }else if((recv_buf[0]==0xcc) && \
              	      (recv_buf[1]==0x01) && \
	      	      (recv_buf[2]==0x01) && \
              	      (recv_buf[3]==0x30) && \
              	      (recv_buf[4]==0x30) && \
              	      (recv_buf[5]==0x31) && \
              	      (recv_buf[6]==0x31)){
		  return 8;//define close status 8
             }
	     	
         }else{
           if(	      (recv_buf[0]==0xcc) && \
              	      (recv_buf[1]==0x02) && \
	      	      (recv_buf[2]==0x02) && \
              	      (recv_buf[3]==0x30) && \
              	      (recv_buf[4]==0x30) && \
              	      (recv_buf[5]==0x30) && \
              	      (recv_buf[6]==0x30)){
		  return 7;//define open satus 7
             }else if((recv_buf[0]==0xcc) && \
              	      (recv_buf[1]==0x02) && \
	      	      (recv_buf[2]==0x02) && \
              	      (recv_buf[3]==0x30) && \
              	      (recv_buf[4]==0x30) && \
              	      (recv_buf[5]==0x31) && \
              	      (recv_buf[6]==0x31)){
		  return 8;//define close status 8
             }

         }
    } 


    return 0;
		
}

int com_send_to_roadblock(char * com_name)
{
    int fd;                            //\u6587\u4ef6\u63cf\u8ff0\u7b26
    int err;                           //\u8fd4\u56de\u8c03\u7528\u51fd\u6570\u7684\u72b6\u6001
    int len;
//    unsigned char send_buf1[] = {0X01,0x06,0x00,0x01,0x00,0x00, 0xd8 , 0x0a};
//    unsigned char send_buf[] = {0X01,0x06,0x00,0x01,0xff,0xff, 0xd9 , 0xba};
    char send_buf[] = {0xaa,0x01,0x01,0x33,0x33};

    fd = UART0_Open(fd,com_name); //\u6253\u5f00\u4e32\u53e3\uff0c\u8fd4\u56de\u6587\u4ef6\u63cf\u8ff0\u7b26
//    fd = UART0_Open(fd,"/dev/ttymxc1");
    err = UART0_Init(fd,9600,0,8,1,'N');
    if (-1 == err || 0 >= fd){
        printf("Set Port Error!\n");
        UART0_Close(fd);
        return 0;
    }

    len = UART0_Send(fd,send_buf,5);
    if(len > 0)
        printf("%s:send data successful,len = %d!\n",com_name,len);
    else
        printf("%s:send data failed!\n",com_name);
    UART0_Close(fd);
    return 1;
}
/*
int com_recv_from_inductor(char * com_name)
{
    int fd;                            //\u6587\u4ef6\u63cf\u8ff0\u7b26
    int err;                           //\u8fd4\u56de\u8c03\u7528\u51fd\u6570\u7684\u72b6\u6001
    int len;
    char rcv_buf[100];
    printf("this is com:%s!\n",com_name);
    fd = UART0_Open(fd,com_name); //\u6253\u5f00\u4e32\u53e3\uff0c\u8fd4\u56de\u6587\u4ef6\u63cf\u8ff0\u7b26
    err = UART0_Init(fd,9600,0,8,1,'N');
    UART0_Close(fd);
    return 0;
    if (-1 == err || 0 >= fd){
//        printf("Set Port Error!\n");
        UART0_Close(fd);
        return 0;
    }
    else
//        printf("com_name %s is success!\n");
    len = UART0_Recv(fd, rcv_buf,20);
    UART0_Close(fd);
    if(len > 0)
    {
        rcv_buf[len] = '\0';
        printf("receive data is %s!len = %d!\n",rcv_buf,len);
        return 1;
    }
    else
        printf("cannot receive data\n");
        return 0;

    if (rcv_buf[0] = '1')
        return 1;
    else
        return 0;
}
*/


