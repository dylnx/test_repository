#include "common_sz.h"
#include "tcputil.h"
#include "socket_client.h"
#include "basic_func.h"
#include "whitelist.h"
#include "log.h"

#define TIME_OUT_TIME 10

//获取当前通行记录发送缓冲区数据量
unsigned int get_sendbuf_count()
{
    unsigned int read_pos;
    unsigned int write_pos;
    unsigned int send_count;

    pthread_mutex_lock(&g_send_info->mutex_send_info);

    write_pos = g_send_info->write_pos;
    read_pos  = g_send_info->read_pos;

    if( write_pos >= read_pos){
        send_count = write_pos - read_pos;
    }else{
        send_count = MAX_SEND_INFO_NUM - read_pos + write_pos;
    }

    pthread_mutex_unlock(&g_send_info->mutex_send_info);

    if(send_count <= 0){
       return 0;
    }
}

bool client_send_operinfo()
{
    int conn_ret;
    int s_count = 0; 
    s_count = get_sendbuf_count();
    if( 0 == s_count) return false; //当前无通行数据

/*  不完善的代码
    int sock = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in service_addr;
    memset(&service_addr,0,sizeof(service_addr));
    service_addr.sin_family = AF_INET;
    service_addr.sin_port = htons(server_port);
    inet_pton(AF_INET,server_ip,&service_addr.sin_addr);
    conn_ret = connect(sock,(struct sockaddr*)&service_addr,sizeof(service_addr));
    if(conn_ret == -1)
    {
        close(sock);
        printf("client:thread_client:cann't connect service!\n");
        print_log(f_sended_server,"client:thread_client:cann't connect server!!!\n");
        return false;
    }
*/

   /*
	server_ip:从配制文件获取IP的全局变量
	server_port:从配制文件获取PORT的全局变量
        5000:建立连接超时时间

   */
   int sock = ConnectTcpServerNonBlock(server_ip,server_port,5000); 
   if( -1 == sock ){
        print_log(f_sended_server,"client cann't connect server!!!\n");
	return false;
	
   }else if( -2 == sock ){
        print_log(f_sended_server,"client connect server timeout!!!\n");
	return false;
   }


    // send info to service
    char send_buf[400];
    int i = 0,k = 0,copy_num = 0;
    int ret_num;
    memset(send_buf,0,400);
    send_buf[0] = '1';
   


    pthread_mutex_lock(&g_send_info->mutex_send_info);
    s_count = get_sendbuf_count();
    if( 0 == s_count){
       //一定要解锁
       pthread_mutex_unlock(&g_send_info->mutex_send_info);

       //当前无通行数据要发送
       return false;     
     }

    for(i = 0; i < s_count; i++)
    {
        send_buf[i*39+1] = '$';
        send_buf[i*39+2] = g_send_info->send_info[g_send_info->read_pos].gate_id/10 + '0';
        send_buf[i*39+3] = g_send_info->send_info[g_send_info->read_pos].gate_id%10 + '0';
        str_assign_value(g_send_info->send_info[g_send_info->read_pos].TID,&send_buf[i*39+4],TID_LEN);
        send_buf[i*39+20] = (g_send_info->send_info[g_send_info->read_pos].be_enter?1:0) + '0';
        str_assign_value(g_send_info->send_info[g_send_info->read_pos].operate_time,&send_buf[i*39+21],19);
        g_send_info->read_pos++;
        g_send_info->read_pos %= MAX_SEND_INFO_NUM;
        
    }
    pthread_mutex_unlock(&g_send_info->mutex_send_info);

    if( s_count > 0 )
    {
       ret_num = send(sock,send_buf,396,MSG_WAITALL);
       if(ret_num>0){
         print_log(f_sended_server,"send to server datas:\n");
         print_log(f_sended_server,"%s\n",send_buf);
       }else if(0 == ret_num){
         print_log(f_sended_server,"send to server failed!!!\n");
       }
    }

    // before close
    send_buf[0] = '4';
    ret_num = send(sock,send_buf,1,MSG_WAITALL);
    usleep(50000);

    //有疑问，因为不确定当前协议发送
    //缓冲中的数据是否已发完，这时关闭可能会丢数据
    close(sock);    
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @功能  接收白名单
 *
 * @返回值   -1 : 失败  0: 成功
 */
/* ----------------------------------------------------------------------------*/
bool client_recv_whitelist1()
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
	recv_buff = (char*)malloc(item_count*item_size);
	int  i =0;
	for(;i<item_count;i++)
	{
		ret = TcpRecvData(fd, recv_buff+i*item_size, item_size, recv_timeout*1000);
		if( ret!=item_size )
		{
			printf("recv wl item[%d] failure\n", i);
			break;
		}
	}
	if( i!=item_count )
	{
		printf("wl item not ready\n");
		free(recv_buff);
		goto error;
	}
	printf("wl item %d recevied\n", item_count);
	
	// save the whiltelist
	ret = RefreshWLDatabase(recv_buff, item_count);
	printf("Update WL Ret %d\n", ret);
	
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



bool client_recv_whitelist()
{
    int conn_ret;
    int sock = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in service_addr;
    memset(&service_addr,0,sizeof(service_addr));
    service_addr.sin_family = AF_INET;
    service_addr.sin_port = htons(server_port);
    inet_pton(AF_INET,server_ip,&service_addr.sin_addr);

    conn_ret = connect(sock,(struct sockaddr*)&service_addr,sizeof(service_addr));
    if(conn_ret == -1)
    {
        close(sock);
        printf("client_whitelist:cann't connect service!\n");
        return false;
    }


    // send single request white list
    char send_buf[4];
    send_buf[0] = '2';
    extern int jointcompute_id;
    send_buf[1] = jointcompute_id/10 + '0';
    send_buf[2] = jointcompute_id%10 + '0';
    send_buf[3] = 0;
    int send_num = send(sock,send_buf,3,MSG_WAITALL);

    printf("client_whitelist:request for white list!\n");

    int len_per_item = 2+TID_LEN+2+CAR_NUM_LEN;
    char recv_buf[MAX_WHITELIST_NUM * len_per_item];
    int rs = 1,recv_num = 0,temp_num = 0,per_num = len_per_item*20,all_num = 0;
    temp_num = recv(sock,&recv_buf[0],6,0);
    bool ret_operate = false;
    int kk = 0;
    for (kk = 0;kk < 6; kk++)
        all_num = all_num * 10 + recv_buf[kk] - '0';
    if (all_num <= 0)
    {
        // before close
        send_buf[0] = '4';
        send_num = send(sock,send_buf,1,MSG_WAITALL);
        usleep(50000);
        close(sock);
        return false;
    }

    while(rs)
    {
        temp_num = 0;
        temp_num = recv(sock,&recv_buf[recv_num],per_num,0);
        if (temp_num == per_num)
            rs = 1;
        else
            rs = 0;
        recv_num += temp_num;
        //printf("client_whitelist:recv all %d:cur_whilt_list num %d!!!\n",recv_num,temp_num);
    }
    recv_buf[recv_num] = '\0';
    print_log(f_sync_whitelist,"recv %d Bytes:\n",recv_num);
    print_log(f_sync_whitelist,"%s\n",recv_buf);
    
    if (recv_num != all_num)
    {
        // before close
        send_buf[0] = '4';
        send_num = send(sock,send_buf,1,MSG_WAITALL);
        usleep(50000);
        close(sock);
        return false;
    }

    int new_item_num = recv_num/len_per_item;
    int ii = 0,list_index = -1;
    char * pBuf;
    int len_item = TID_LEN + 2 + CAR_NUM_LEN;
    char item[len_item];
    pthread_mutex_lock(&g_white_list->mutex_white_list);
    for (ii = 0;ii < new_item_num;ii++)
    {
        pBuf = &recv_buf[ii*len_per_item];
        str_assign_value(&pBuf[2],item,len_item);
        switch(pBuf[1])
        {
        case '1':   // add
            ret_operate = add_white_list(item,len_item);
            if( true == ret_operate ){
               print_log(f_sync_whitelist,"add the %s successfully!!!\n",item);
            }else{
               print_log(f_sync_whitelist,"found the %s in whitelist,add failed!!!\n",item);
            }
            break;
        case '2':   // delete
            ret_operate = del_white_list(item,len_item);
            if( true == ret_operate ){
                print_log(f_sync_whitelist,"delete the %s successfully!!!\n",item);
            }else{
                print_log(f_sync_whitelist,"whitelist empty or couldn't find  the %s,delete failed!!!\n",item);
            }
            break;
        case '3':   // edit
            ret_operate = edit_white_list(item,len_item);
            if( true == ret_operate ){
                print_log(f_sync_whitelist,"edit the %s successfully!!!\n",item);
            }else{
                print_log(f_sync_whitelist,"whitelist empty or couldn't find  the %s,delete failed!!!\n",item);
            }
            break;
        default:
            break;
        }
    }

    pthread_mutex_unlock(&g_white_list->mutex_white_list);

    // write into white list
    file_write_whitelist();

    send_buf[3] = '1';
    send_num = send(sock,send_buf,4,MSG_WAITALL);

    // before close
    send_buf[0] = '4';
    send_num = send(sock,send_buf,1,MSG_WAITALL);
    close(sock);

    return true;
}

int client_init_socket()
{
    int sock = socket(AF_INET,SOCK_STREAM,0);
    int conn_ret;
    struct sockaddr_in service_addr;
    memset(&service_addr,0,sizeof(service_addr));
    service_addr.sin_family = AF_INET;
    service_addr.sin_port = htons(33002);
    inet_pton(AF_INET,"192.168.19.100",&service_addr.sin_addr);
    conn_ret = connect(sock,(struct sockaddr*)&service_addr,sizeof(service_addr));
    if(conn_ret == -1)
    {
        close(sock);
        sock = -1;
    }
    return sock;

}

bool client_camera(int sock)
{
//    int len_date = 4 + 4 + sizeof(TPLATE_INFO_MY);
    int len_heart = 4 + 4 + 16;

    int len_max_pack = 300;

    int temp_recv_num = 0;
    unsigned char recv_buf[len_max_pack];
    memset(recv_buf,0,len_max_pack);

    // recv head
    char head[] = {0xaa,0x55,0xaa,0x55};
    char date_heart[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    char send_buf[] = {0xaa,0x55,0xaa,0x55,0x10,0x00,0x00,0x00,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    temp_recv_num = recv(sock,recv_buf,4,0);
    if (!str_equal(head,recv_buf,4))
        return false;
    memset(recv_buf,0,len_max_pack);

//    send(sock,send_buf,sizeof(send_buf),MSG_WAITALL);
    // recv len_num
    temp_recv_num = recv(sock,recv_buf,4,0);
    int cur_all_num = 0,all_num = 0;
    all_num = recv_buf[0] + recv_buf[1]*256 + recv_buf[2]*256*256 + recv_buf[3]*256*256*256;
    memset(recv_buf,0,len_max_pack);

    char chepai[13];

    // recv date
    if (all_num == 16)
    {   // recv heart pocket
        temp_recv_num = recv(sock,recv_buf,len_max_pack,0);
        printf("camera: recv %d chars!!!!!\n",temp_recv_num);
        if (str_equal(date_heart,recv_buf,16))
        {
            temp_recv_num = send(sock,send_buf,sizeof(send_buf),MSG_WAITALL);
            return true;
        }
        else
            return false;
    }
    else
    {
        // recv data packet
        int temp_num = 0,per_num = len_max_pack,count = 0;
        while(cur_all_num != all_num)
        {
            temp_num = 0;
            temp_num = recv(sock,recv_buf,per_num,0);
            if (count == 0)
            {
                count ++;
                memcpy(chepai,&recv_buf[10],13);
            }
            if (temp_num != 0)
            {
                cur_all_num += temp_num;
                per_num = (all_num - cur_all_num < len_max_pack)?(all_num - cur_all_num):len_max_pack;
            }
            else if(cur_all_num != all_num)
                return false;
        }
    }
    // date pocket
    printf("camera: success recv packet all_num:%d cur_all_num:%d chars!!!!!!!!!!!\n",all_num,cur_all_num);
    memset(recv_buf,0,len_max_pack);

    if (str_equal("NoPlate",chepai,7))
        return true;

    char chepai_utf8[13];
    gb2312_unicode(chepai,strlen(chepai),chepai_utf8,13);
    printf("%s!\n",chepai_utf8);

    // whether is the car in white_list ?
    bool be_in_white_list = false;
    pthread_mutex_lock(&g_white_list->mutex_white_list);
    int kk_in_white = 0;
    for (kk_in_white = 0; kk_in_white < g_white_list->white_list_num; kk_in_white++)
    {
        if(str_equal(chepai_utf8,&g_white_list->white_list[kk_in_white][TID_LEN + 2],CAR_NUM_LEN))
            break;
    }
    if (kk_in_white < g_white_list->white_list_num)
        be_in_white_list = true;
    else
        be_in_white_list = false;
    pthread_mutex_unlock(&g_white_list->mutex_white_list);
    if (be_in_white_list)
        return true;

    // make led show info
    pthread_mutex_lock(&g_show_camera->mutex_led_show_camera);
    if (g_show_camera->led_info_camera_num < LED_SHOW_MAX_NUM)
    {
        str_assign_value(gates[0].led_ip,g_show_camera->led_ip,15);
        g_show_camera->led_port = gates[0].led_port;
        str_assign_value(chepai_utf8,g_show_camera->led_show_camera[g_show_camera->led_info_camera_num].car_num,CAR_NUM_LEN);
        g_show_camera->led_show_camera[g_show_camera->led_info_camera_num].be_entry = false;
        g_show_camera->led_info_camera_num += 1;
    }
    pthread_mutex_unlock(&g_show_camera->mutex_led_show_camera);
    return true;
}
