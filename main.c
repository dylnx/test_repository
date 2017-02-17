#include <syscall.h>
#include <unistd.h>
#include <signal.h>
#include "common_sz.h"
#include "basic_func.h"
#include "controler.h"
#include "serial.h"
#include "gpio.h"
#include "cond_thread.h"
#include "whitelist.h"
#include "logfile.h"

bool initial(void);
int create_pidfile(const char *file);

struct CondThread *g_PassRecordLogWriteFileThread;
struct CondThread *g_PassRecordSendThread;
struct CondThread *g_LedRecordSendThread;

int main(void)
{
    int i,s = 0,thread_arg,ret_createmtx = 0;
    void * ret;

    //将当前进程后台化
    //daemon(1,1);
    //create_pidfile("ReadTags.pid");
    signal(SIGPIPE,SIG_IGN);


    //初始化全局日志
    f_sysinit = open_log_file("./log/sysinit.log");
    f_misc_running = open_log_file("./log/miscrunning.log");
    f_error = open_log_file("./log/timeouterror.log");
    f_passed_success = open_log_file("./log/passedsuccess.log");
    f_passed_failed = open_log_file("./log/passedfailed.log");
    f_sended_server = open_log_file("./log/sendedserver.log");
    f_sync_whitelist = open_log_file("./log/syncwhitelist.log");
    print_log(f_sysinit,"Program Started!!!");    

    //读config.ini文件中的配制数据信息
    bool bRet = initial();
    if (!bRet)
    {
        printf("init error!\n");
        return -1;
    }


    /******************************************************************/
    //初始化白名单数据库功能 
    WhiteListDatabaseInit();
    /******************************************************************/
	

    /******************************************************************/
    //创建通行记录写日志线程,将内存队列中的通行记录,写入断链续传日志,
    //PassRecordLogHandle函数负责上述功能.
    InitPassRecordLogFile();
    g_PassRecordLogWriteFileThread = CreateCondThread(
				PassRecordLogHandle);
    ThreadRun(g_PassRecordLogWriteFileThread);
    /*****************************************************************/


    /*****************************************************************/
    //创建通行记录实时上传线程,并将当前通行记录,包括发送成功或失败(成功
    //标记0，失败标记1）写入队列后通知写日志线程,
    //PassRecordSendHandle函数负责上述功能.
    g_PassRecordSendThread = CreateCondThread(
				PassRecordSendHandle);
    ThreadRun(g_PassRecordSendThread);
    /*****************************************************************/


    /*****************************************************************/
    //创建LED显示线程，与上面方式同样是采用条件变量队列方式.
    //LedRecordSendHandle函数负责显示到屏幕.
    if(!strcmp(exist_led,"YES"))
    {
	    g_LedRecordSendThread = CreateCondThread(
					LedRecordSendHandle);
	    ThreadRun(g_LedRecordSendThread);

    }
    /*****************************************************************/



    


    g_tags_array = (PTAGOBJ)calloc(TAG_NUM_MAX,sizeof(TAGOBJ));
    if(NULL == g_tags_array)
    {
        printf("Error: malloc g_tags_array  faild!!\n");
        print_log(f_sysinit,"Error: malloc g_tags_array  faild!!");
        return 1;
    }

    g_old_passed_array = (POLDPASSEDOBJ)calloc(1,sizeof(OLDPASSEDOBJ)* TAG_OLD_PASSED_MAX);
    if( NULL == g_old_passed_array )
    {
        printf("Error: calloc g_old_passed_array  faild!!\n");
        print_log(f_sysinit,"Error: calloc g_old_passed_array  faild!!\n");
        return 1;//exit the program
    }


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
        return 1;//exit the program
    }


    

    if(!strcmp(door_open_method,"RELAY"))
    {
	 //初始化GPIO
	 init_gpio();
    }


    if(!strcmp(door_open_method,"RS485"))
    {
	    for(i=0;i<gate_num;i++)
	    {
		gates[i].com_roadblock_fd = com_init(gates[i].com_roadblock);
		//printf("com%d fd=%d\n",i,gates[i].com_roadblock_fd);
		if(gates[i].com_roadblock_fd==0){
		    print_log(f_sysinit,"ERROR!!!Dev:%s init failed!!!\n",gates[i].com_roadblock);
		}
		    usleep(100*1000);
	    }
    }

    //create some thread
    pthread_t thread_control,thread_request_whitelist,	\
    thread_resend_passrecord,thread_kill_fork,thread_camera;


    s = pthread_create(&thread_request_whitelist,NULL,(void *)&ThreadRequestWhitelist,NULL);
    if(s != 0)
    {
	print_log(f_sysinit,"pthread_create thread_request_whitelist faild!!");
 	exit(EXIT_FAILURE);		
    }
    usleep(500*1000);
    
    s = pthread_create(&thread_resend_passrecord,NULL,(void *)&ThreadResendPassrecord,NULL);
    if(s !=0)
    {
	print_log(f_sysinit,"pthread_create thread_resend_passrecord faild!!");
 	exit(EXIT_FAILURE);		
    }


    s =	pthread_create(&thread_control,NULL,(void *)&ThreadMonitorCapDeal,NULL);
    if( 0 != s ){
	print_log(f_sysinit,"pthread_create Control thread faild!!");
 	exit(EXIT_FAILURE);		
    }
    usleep(500*1000);
	
    s = pthread_create(&thread_kill_fork,NULL,(void *)&ThreadKillFork,NULL);
    if( 0 != s ){
	print_log(f_sysinit,"pthread_create faild!!");
 	exit(EXIT_FAILURE);		
    }
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
    //读取服务器IP
    if (!readStringParam(buffer,buf_len, "server_ip",server_ip))
    {
    	print_log(f_sysinit,"read server_ip failed! exit the program!!!\n");    
 	exit(EXIT_FAILURE);		
    }
    print_log(f_sysinit,"server_ip=%s",server_ip);    

    //读取服务器PORT
    if (!readIntParam(buffer,buf_len, "server_port",&server_port))
    {
    	print_log(f_sysinit,"read server_port failed! exit the program!!!\n");    
 	exit(EXIT_FAILURE);		
    }
    print_log(f_sysinit,"server_port=%d",server_port);    

    //读取读写器IP
    if (!readStringParam(buffer,buf_len, "reader_ip",reader_ip))
    {
    	print_log(f_sysinit,"read reader_ip failed! exit the program!!!\n");    
 	exit(EXIT_FAILURE);		
    }
    print_log(f_sysinit,"reader_ip=%s",reader_ip);    

    //读取读写器PORT
    if (!readIntParam(buffer,buf_len, "reader_port",&reader_port))
    {
    	print_log(f_sysinit,"read reader_port failed! exit the program!!!\n");    
 	exit(EXIT_FAILURE);		
    }
    print_log(f_sysinit,"reader_port=%d",reader_port);    

    if (!readIntParam(buffer,buf_len, "reader_rate",&reader_rate))
    {
        // log
        return false;
    }
    print_log(f_sysinit,"reader_rate=%d",reader_rate);    

    if (!readIntParam(buffer,buf_len, "inductor_signal_keep_time",&inductor_signal_keep_time))
    {
        // log
        return false;
    }
    print_log(f_sysinit,"inductor_signal_keep_time=%d",inductor_signal_keep_time);    

    if (!readIntParam(buffer,buf_len, "passrecord_resend_inteval",&passrecord_resend_inteval))
    {
        // log
        return false;
    }
    print_log(f_sysinit,"passrecord_resend_inteval=%d",passrecord_resend_inteval);    

    if (!readIntParam(buffer,buf_len, "passrecord_resend_limit_time",&passrecord_resend_limit_time))
    {
        // log
        return false;
    }
    print_log(f_sysinit,"passrecord_resend_limit_time=%d",passrecord_resend_limit_time);    

    if (!readIntParam(buffer,buf_len, "request_whitelist_inteval",&request_whitelist_inteval))
    {
        // log
        return false;
    }
    print_log(f_sysinit,"request_whitelist_inteval=%d",request_whitelist_inteval);    

    if (!readIntParam(buffer,buf_len, "road_block_count",&g_road_block_count))
    {
        // log
        return false;
    }
    print_log(f_sysinit,"road_block_count=%d",g_road_block_count);  

    if (!readIntParam(buffer,buf_len, "gate_num",&gate_num))
    {
        // log
        return false;
    }
    print_log(f_sysinit,"gate_num=%d",gate_num);    




    if (!readStringParam(buffer,buf_len, "working_way",working_way))
    {
        // log
        return false;
    }
    print_log(f_sysinit,"working_way=%s",working_way);    

    if(!strcmp(working_way,"INDUCTION"))
    {
      if(!readIntParam(buffer,buf_len, "inteval_induction",
	&get_tags_inteval)){
		// log
		return false;
	}
    	print_log(f_sysinit,"get_tags_inteval=%d",get_tags_inteval);    
    }else{//working_way="POLL",系统采用轮询工作方式

	if(!readIntParam(buffer,buf_len, "inteval_poll",
	&get_tags_inteval)){
		// log
		return false;
	}   
    	print_log(f_sysinit,"get_tags_inteval=%d",get_tags_inteval);    
    }

    if (!readStringParam(buffer,buf_len, "door_open_method",door_open_method))
    {
        // log
        return false;
    }
    print_log(f_sysinit,"door_open_method=%s",door_open_method);    

    if (!readIntParam(buffer,buf_len, "log_file_size",&log_size))
    {
        // log
        return false;
    }
    log_size *= 1024 *1024;//read from config.ini 


    if (!readIntParam(buffer,buf_len, "dev_id",&dev_id))
    {
        // log
        return false;
    }
    print_log(f_sysinit,"dev_id=%d",dev_id);    

    if (!readStringParam(buffer,buf_len, "exist_led",exist_led))
    {
        // log
        return false;
    }
    print_log(f_sysinit,"exist_led=%s\n",exist_led);

    
      

    int i = 0;
    char key_name[30];
    for (i=0;i<gate_num;i++)//two gates,one for enter,anther for leave.
    {
        char ss[3];

    	print_log(f_sysinit,"************gate[%d] spec paramaters*************\n",i);    

        sprintf(key_name,"%s_%d_%s","gate_info",i,"type");
        readStringParam(buffer,buf_len, key_name,ss);
        gates[i].gate_type = ss[0];
    	print_log(f_sysinit,"gate_info_%d_type=%c",i,gates[i].gate_type);    

        sprintf(key_name,"%s_%d_%s","gate_info",i,"id");
        readIntParam(buffer,buf_len, key_name,&gates[i].gate_id);
    	print_log(f_sysinit,"gate_info_%d_id=%d",i,gates[i].gate_id);    
	
	sprintf(key_name,"%s_%d_%s","gate_info",i,"inductor_gpio");
        readIntParam(buffer,buf_len, key_name,&gates[i].inductor_gpio);
    	print_log(f_sysinit,"gate_info_%d_inductor_gpio=%d",i,gates[i].inductor_gpio);    

	sprintf(key_name,"%s_%d_%s","gate_info",i,"relay_gpio");
        readIntParam(buffer,buf_len, key_name,&gates[i].relay_gpio);
    	print_log(f_sysinit,"gate_info_%d_relay_gpio=%d",i,gates[i].relay_gpio);    

        sprintf(key_name,"%s_%d_%s","gate_info",i,"roadblock");
        readStringParam(buffer,buf_len, key_name,gates[i].com_roadblock);
    	print_log(f_sysinit,"gate_info_%d_roadblock=%s",i,gates[i].com_roadblock);    

        sprintf(key_name,"%s_%d_%s","gate_info",i,"led_ip");
        readStringParam(buffer,buf_len, key_name,gates[i].led_ip);
    	print_log(f_sysinit,"gate_info_%d_led_ip=%s",i,gates[i].led_ip);    

        sprintf(key_name,"%s_%d_%s","gate_info",i,"led_port");
        readIntParam(buffer,buf_len, key_name,&gates[i].led_port);
    	print_log(f_sysinit,"gate_info_%d_led_port=%d",i,gates[i].led_port);    

        sprintf(key_name,"%s_%d_%s","gate_info",i,"right");
        readStringParam(buffer,buf_len, key_name,gates[i].gate_rights);
    	print_log(f_sysinit,"gate_info_%d_right=%s",i,gates[i].gate_rights);    

        sprintf(key_name,"%s_%d","ant_num",i);
        readIntParam(buffer,buf_len, key_name,&gates[i].ant_num);
    	print_log(f_sysinit,"gate_info_%d_antnum=%d",i,gates[i].ant_num);    

        sprintf(key_name,"%s_%d","gate_ant",i);
        readStringParam(buffer,buf_len, key_name,gates[i].ants);
    	print_log(f_sysinit,"gate_ant_%d=%s\n\n",i,gates[i].ants);    
    }

   
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
