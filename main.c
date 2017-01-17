#include <syscall.h>
#include <unistd.h>
#include <signal.h>
#include "common_sz.h"
#include "basic_func.h"
#include "threads_sz.h"
#include "serial.h"
#include "gpio.h"
#include "cond_thread.h"
#include "logfile.h"

bool initial(void);
bool get_white_list(void);
bool get_tempcar_list(void);
int create_pidfile(const char *file);

struct CondThread *g_PassRecordLogWriteFileThread;
struct CondThread *g_PassRecordSendThread;

int main(void)
{
    int i,s = 0,thread_arg,ret_createmtx = 0;
    void * ret;

    daemon(1,1);
   // create_pidfile("ReadTags.pid");
    signal(SIGPIPE,SIG_IGN);

    //init gpio
    init_gpio();
	
    // Init the pass record log file
    InitPassRecordLogFile();
    g_PassRecordLogWriteFileThread = CreateCondThread(
		PassRecordLogHandle);
    ThreadRun(g_PassRecordLogWriteFileThread);

    g_PassRecordSendThread = CreateCondThread(
		PassRecordSendHandle);
    ThreadRun(g_PassRecordSendThread);

    //init log
    f_sysinit = open_log_file("./log/sysinit.log");
    f_misc_running = open_log_file("./log/miscrunning.log");
    f_error = open_log_file("./log/timeouterror.log");
    f_passed_success = open_log_file("./log/passedsuccess.log");
    f_passed_failed = open_log_file("./log/passedfailed.log");
    f_sended_server = open_log_file("./log/sendedserver.log");
    f_sync_whitelist = open_log_file("./log/syncwhitelist.log");

    print_log(f_sysinit,"Program Started!!!");    


    g_tags_array = (PTAGOBJ)calloc(TAG_NUM_MAX,sizeof(TAGOBJ));
    if(NULL == g_tags_array)
    {
        printf("Error: malloc g_tags_array  faild!!\n");
        print_log(f_sysinit,"Error: malloc g_tags_array  faild!!");
        return 1;
    }

    //memset(g_tags_array,0,sizeof(TAG_OBJ) * TAG_NUM_MAX);
    //sleep 50ums
    usleep(50*1000);

    g_tag_flag = (char *)calloc(TAG_NUM_MAX,sizeof(char));
    if(NULL == g_tag_flag)
    {
        printf("Error: malloc g_tag_flag  faild!!\n");
        print_log(f_sysinit,"Error: malloc g_tag_flag  faild!!");
        return 1;
    }
    //memset(g_tag_flag,1,sizeof(char)*TAG_NUM_MAX);
    //sleep 50ums
    usleep(50*1000);

    g_old_passed_array = (POLDPASSEDOBJ)calloc(1,sizeof(OLDPASSEDOBJ)* TAG_OLD_PASSED_MAX);
    if( NULL == g_old_passed_array )
    {
        printf("Error: calloc g_old_passed_array  faild!!\n");
        print_log(f_sysinit,"Error: calloc g_old_passed_array  faild!!\n");
        return 1;//exit the program
    }
    //memset(g_old_passed_array,0,sizeof(OLDPASSEDOBJ) * TAG_OLD_PASSED_MAX);
    //sleep 50ums
    usleep(50*1000);


    g_led_show_list = (PLED_SHOW_LIST)calloc(2, sizeof(LED_SHOW_LIST));
    if( NULL == g_led_show_list )
    {
        printf("Error: calloc g_led_show_list  faild!!\n");
        print_log(f_sysinit,"Error: calloc g_led_show_list  faild!!\n");
    }
    //memset(g_led_show_list,0,sizeof(LED_SHOW_LIST) * 2);

    //two gates,two led
    for( i = 0;i<2;i++ ){
       ret_createmtx = pthread_mutex_init(&g_led_show_list[i].mutex_led_show,NULL); 
       if( 0 != ret_createmtx){
            print_log(f_sysinit,"ERROR!!!pthread_mutex_init g_led_show_list[%d].mutex_led_show",i);
       }
       str_assign_value(gates[i].led_ip, \
       g_led_show_list[i].led_ip,15); 

       g_led_show_list[i].led_port = gates[i].led_port; 
       
    }
    //sleep 50ums
    usleep(50*1000);


    //alloc white list buffer and init it
    g_white_list = (PWHITELIST)calloc(1,sizeof(WHITELIST));
    if( NULL == g_white_list )
    {
        printf("Error: calloc g_white_list  faild!!\n");
        print_log(f_sysinit,"Error: calloc g_white_list  faild!!\n");
        return 1;//exit the program
    }
    ret_createmtx = pthread_mutex_init(&g_white_list->mutex_white_list,NULL); 
    if( 0 != ret_createmtx){
        print_log(f_sysinit,"ERROR!!!pthread_mutex_init g_white_list->mutex_white_list");
    }
    //memset(g_white_list,0,sizeof(WHITELIST));
    //sleep 50ums
    usleep(50*1000);


    //alloc operate info  buffer and init it
    g_operate_info = (POPERATEINFO)calloc(1,sizeof(OPERATEINFO));
    if( NULL == g_operate_info )
    {
        printf("Error: calloc g_operate_info  faild!!\n");
        print_log(f_sysinit,"Error: calloc g_operate_info  faild!!\n");
        return 1;//exit the program
    }
    ret_createmtx = pthread_mutex_init(&g_operate_info->mutex_operate_info,NULL); 
    if( 0 != ret_createmtx){
        print_log(f_sysinit,"ERROR!!!pthread_mutex_init g_operate_info->mutex_operate_info");
    }
    //memset(g_operate_info,0,sizeof(OPERATEINFO));
    //sleep 50ums
    usleep(50*1000);

    //glloc send info buffer and init it
    g_send_info = (PSENDINFO)calloc(1,sizeof(SENDINFO));
    if( NULL == g_send_info )
    {
        printf("Error: calloc g_send_info  faild!!\n");
        print_log(f_sysinit,"Error: calloc g_send_info  faild!!\n");
        return 1;//exit the program
    }
    ret_createmtx = pthread_mutex_init(&g_send_info->mutex_send_info,NULL); 
    if( 0 != ret_createmtx){
        print_log(f_sysinit,"ERROR!!!pthread_mutex_init g_send_info->mutex_send_info");
    }
    //memset(g_send_info,0,sizeof(SENDINFO));
    //sleep 50ums
    usleep(50*1000);


    //alloc show camera  buffer and init it
    g_show_camera = (PSHOWCAMERA)calloc(1,sizeof(SHOWCAMERA));
    if( NULL == g_show_camera )
    {
        printf("Error: calloc g_show_camera  faild!!\n");
        print_log(f_sysinit,"Error: calloc g_show_camera  faild!!\n");
        return 1;//exit the program
    }
    ret_createmtx = pthread_mutex_init(&g_show_camera->mutex_led_show_camera,NULL); 
    if( 0 != ret_createmtx){
        print_log(f_sysinit,"ERROR!!!pthread_mutex_init g_show_camera->mutex_led_show_camera");
    }
    //memset(g_show_camera,0,sizeof(SHOWCAMERA));
    //sleep 50ums
    usleep(50*1000);


    // read config file; get white list
    bool bRet = initial();
    if (!bRet)
    {
        printf("init error!\n");
        return -1;
    }

#if 1
    for(i=0;i<2;i++)
    {
       	gates[i].com_roadblock_fd = com_init(gates[i].com_roadblock);
	//printf("com%d fd=%d\n",i,gates[i].com_roadblock_fd);
       	if(gates[i].com_roadblock_fd==0){
            print_log(f_sysinit,"ERROR!!!Dev:%s init failed!!!\n",gates[i].com_roadblock);
        }
            usleep(100*1000);
    }
#endif

    //create some thread
    pthread_t thread_control,thread_led0,thread_led1,thread_connect_whitelist,	\
    thread_info_to_server,thread_kill_fork,thread_camera;

#if 1 
    thread_arg = 0;
    s = pthread_create(&thread_led0,NULL,(void *)&ThreadLedShow,&thread_arg);
    if(s != 0)
    {
	print_log(f_sysinit,"pthread_create showLed0 thread faild!!");
    }
#endif

    //sleep 500ums
    usleep(500*1000);

#if 1 
    thread_arg = 1;
    s = pthread_create(&thread_led1,NULL,(void *)&ThreadLedShow,&thread_arg);
    if(s != 0)
    {
	print_log(f_sysinit,"pthread_create showLed1 thread faild!!");
    }
#endif

    //sleep 500ums
    usleep(500*1000);

    s = pthread_create(&thread_connect_whitelist,NULL,(void *)&ThreadConnectWhitelist,NULL);
    if(s != 0)
    {
	print_log(f_sysinit,"pthread_create thread_connect_whitelist faild!!");
 	exit(EXIT_FAILURE);		
    }

    //sleep 500ums
    usleep(500*1000);

    s = pthread_create(&thread_info_to_server,NULL,(void *)&ThreadInfoToServer,NULL);
    if(s != 0)
    {
	print_log(f_sysinit,"pthread_create thread_info_to_server faild!!");
 	exit(EXIT_FAILURE);		
    }

    //sleep 500ums
    usleep(500*1000);

#if 0
    s = pthread_create(&thread_camera,NULL,(void *)&ThreadCamera,NULL);
    if( 0 != s ){
 	exit(EXIT_FAILURE);		
    }

    //sleep 500ums
    usleep(500*1000);
#endif

    s =	pthread_create(&thread_control,NULL,(void *)&ThreadMonitorCapDeal,NULL);
    if( 0 != s ){
	print_log(f_sysinit,"pthread_create Control thread faild!!");
 	exit(EXIT_FAILURE);		
    }

    //sleep 500ums
    usleep(500*1000);
	
    s = pthread_create(&thread_kill_fork,NULL,(void *)&ThreadKillFork,NULL);
    if( 0 != s ){
	print_log(f_sysinit,"pthread_create faild!!");
 	exit(EXIT_FAILURE);		
    }

    //sleep 500ums
    usleep(500*1000);

    pthread_join(thread_control,&ret);
    print_log(f_sysinit,"Program Exit!!!");
	
    printf("exit!\n");
    return 1;
}

bool initial(void)
{
    // get config info
    char* filename = "config.ini";
    int intValue;
    char buffer[4096];
    int buf_len = file_read(filename,buffer);

    char value[400];
    if (!readStringParam(buffer,buf_len, "server_ip",server_ip))
    {
        // log
        return false;
    }
    if (!readIntParam(buffer,buf_len, "server_port",&server_port))
    {
        // log
        return false;
    }
    if (!readStringParam(buffer,buf_len, "reader_ip",reader_ip))
    {
        // log
        return false;
    }
    if (!readIntParam(buffer,buf_len, "reader_port",&reader_port))
    {
        // log
        return false;
    }
    if (!readIntParam(buffer,buf_len, "reader_rate",&reader_rate))
    {
        // log
        return false;
    }
    if (!readIntParam(buffer,buf_len, "inductor_signal_keep_time",&inductor_signal_keep_time))
    {
        // log
        return false;
    }
    if (!readIntParam(buffer,buf_len, "dev_id",&jointcompute_id))
    {
        // log
        return false;
    }
    if (!readStringParam(buffer,buf_len, "white_list_path",white_list_path))
    {
        // log
        return false;
    }
    if (!readIntParam(buffer,buf_len, "time_of_update_list",&time_of_update_list))
    {
        // log
        return false;
    }
    
   if (!readIntParam(buffer,buf_len, "log_file_size",&log_file_size))
    {
        // log
        return false;
    }
    log_file_size *= 1024 *1024;//read from config.ini 
    

    int i = 0;
    char key_name[30];
    for (i=0;i<2;i++)//two gates,one for enter,anther for leave.
    {
        char ss[3];
        sprintf(key_name,"%s_%d_%s","gate_info",i,"type");
        readStringParam(buffer,buf_len, key_name,ss);
        gates[i].gate_type = ss[0];
        sprintf(key_name,"%s_%d_%s","gate_info",i,"id");
        readIntParam(buffer,buf_len, key_name,&gates[i].gate_id);
        sprintf(key_name,"%s_%d_%s","gate_info",i,"roadblock");
        readStringParam(buffer,buf_len, key_name,gates[i].com_roadblock);

	printf(" gates[%d].com_roadblock=%s\n",i,gates[i].com_roadblock);

        //sprintf(key_name,"%s_%d_%s","gate_info",i,"inductor");
        //readStringParam(buffer,buf_len, key_name,gates[i].com_inductor);

        sprintf(key_name,"%s_%d_%s","gate_info",i,"led_ip");
        readStringParam(buffer,buf_len, key_name,gates[i].led_ip);
        sprintf(key_name,"%s_%d_%s","gate_info",i,"led_port");
        readIntParam(buffer,buf_len, key_name,&gates[i].led_port);
        sprintf(key_name,"%s_%d_%s","gate_info",i,"right");
        readStringParam(buffer,buf_len, key_name,gates[i].gate_rights);
        sprintf(key_name,"%s_%d","ant_num",i);
        readIntParam(buffer,buf_len, key_name,&gates[i].ant_num);
        sprintf(key_name,"%s_%d","gate_ant",i);
        readIntParam(buffer,buf_len, key_name,&gates[i].ants[0]);
	printf("gates[%d].ant=%d\n",i,gates[i].ants[0]);
    }

    if (!get_white_list())
    {
        printf("get white list error!\n");
        return false;
    }

    return true;
}

bool get_white_list(void)
{
  // extern char white_list_path[100];
  // extern unsigned char white_list[MAX_WHITELIST_NUM][TID_LEN+2+CAR_NUM_LEN];
  // extern pthread_mutex_t mutex_white_list;
  // extern int white_list_num;

    char buf[MAX_WHITELIST_NUM*(TID_LEN+2+CAR_NUM_LEN+5)];

  //  pthread_mutex_init(&mutex_white_list,NULL);

    pthread_mutex_lock(&g_white_list->mutex_white_list);
    int len = file_read(white_list_path,buf);
    int i = 0,k = 0;
    g_white_list->white_list_num = 0;
    //memset(g_white_list->white_list,0,MAX_WHITELIST_NUM*(TID_LEN+2+CAR_NUM_LEN));
    for (i = 0;i < len;i++)
    {
        if (buf[i] == '\n' || buf[i] == '\r')
            k = i+1;
        if (buf[i] == '\n')
            g_white_list->white_list_num++;
        if (buf[i] != '\n' && buf[i] != '\r')
        {
            g_white_list->white_list[g_white_list->white_list_num][i-k] = buf[i]; //"i-k" is set  second index exampe be first [0]
        }
    }
    // no tail
    if (buf[len-1] != '\n')
        g_white_list->white_list_num++;
    // delete tail
    while (g_white_list->white_list[g_white_list->white_list_num-1][0] == 0 && g_white_list->white_list_num > 0)
        g_white_list->white_list_num -- ;

    // delete mid   //let the last date fill to white_list[i] that if (white_list[i][0] == 0),and loop such steps.

    for(i = 0; i < g_white_list->white_list_num; i++)
    {
        if(g_white_list->white_list[i][0] == 0)
        {
            while (g_white_list->white_list[g_white_list->white_list_num-1][0] == 0)
                g_white_list->white_list_num--;
            if(i < g_white_list->white_list_num)
            {
                str_assign_value(g_white_list->white_list[g_white_list->white_list_num-1],g_white_list->white_list[i],TID_LEN+2+CAR_NUM_LEN);
                g_white_list->white_list_num--;
            }
        }
    }
    printf("main:white list num %d!\n",g_white_list->white_list_num);
    pthread_mutex_unlock(&g_white_list->mutex_white_list);

    return true;
}
int create_pidfile(const char *file)
{
    if( file == NULL) {
        exit(-1);
    };
    FILE *fp = fopen(file, "w");
    if( NULL != fp){
        pid_t mypid = syscall(SYS_getpid);
        fprintf(fp, "%d", mypid);
        fclose(fp);
    }
    else{
        perror( "can not open pid file ");
        exit(-1);
    }
}
