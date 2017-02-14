#ifndef SERIAL_H_O3X4X8KD
#define SERIAL_H_O3X4X8KD

int UART0_Open(int fd,char* port);
int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity);
int UART0_Open(int fd,char* port);
int UART0_Recv(int fd, char *rcv_buf,int data_len);
int UART0_Send(int fd, char *send_buf,int data_len);
int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);

int com_init(char * com_name);
int com_open_roadblock(int fd,int roadBlockCnt);
int com_send_to_roadblock(char * com_name);
//int com_recv_from_inductor(char * com_name);

#endif /* end of include guard: SERIAL_H_O3X4X8KD */
