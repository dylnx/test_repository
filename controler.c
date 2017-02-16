#include <unistd.h>     
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      
#include <termios.h>    
#include <errno.h>      
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "common_sz.h"
#include "api.h"
#include "ledapi.h"
#include "gpio.h"
#include "serial.h"
#include "pass_record.h"
#include "whitelist.h"
#include "socket_client.h"



list result;

void before_fork_kill();

unsigned long long  last_heart_time = 0;
unsigned long long  last_signal_time = 0;

int ConnectionReader();
int CheckLandInduction(int *whitchInduction);
int GetTagsAndDeal(int *whitchInduction);



int ConnectionReader()
{
	int retVal=0;
        struct   timeval    timeval;

	while(1)
	{
                if( 0 != reader){
		   close_reader(reader);		
                   reader = 0;
                }

		usleep(100);

		reader = new_reader(reader_ip,reader_port);

		if(0 != reader){
			set_reader_param(reader,PARAM_PROTOCOL,PROTOCOL_ISO6C);
			set_reader_param(reader,PARAM_READMODE,READMODE_LOWSPEED);
			set_reader_param(reader,PARAM_FREQMODE,FREQ_DYN);
			//set_reader_param(reader,PARAM_RF_ANTRRATE,10);

			set_reader_param(reader,PARAM_RF_ANTNUM,ANTENNA_1 | ANTENNA_2);
			set_reader_param(reader,PARAM_RF_ANTPOWER,reader_rate);
			set_reader_param(reader,PARAM_DURTIME,200);

			Command tid = { ISO6C_TID, 2, 4, 0};
			set_read_command1(reader,tid);

			retVal = init_reader(reader);
			if(ERROR == retVal)
			{
				print_log(f_sysinit,"init_reader() ERROR\n!");
				usleep(100*1000);
				continue;
			}else{
				sleep(4);//等待射频模块启动中...
				print_log(f_sysinit,"connected reader successfully!!!");

                                //连接成功后，初始last_heart)time时间，用于心跳判断条件
                                gettimeofday(&timeval, NULL);
                                last_heart_time = (unsigned long long)timeval.tv_sec;

                                break;//连接一旦建立成功，即退出while循环
			}
		}else{
			print_log(f_sysinit,"new_reader() NULL\n!");
			usleep(500*1000);
			continue;
		     }
	} // end of while(.....

	return OK;
}

int CheckLandInduction(int *whitchInduction)
{
	int heart_ret=0;
        int retVal=0;

	unsigned long long  cur_heart_time = 0;
        unsigned long long  cur_signal_time = 0;

        struct   timeval    timeval;

	unsigned int gpio8Val  = 0;
        unsigned int gpio10Val = 0;


	//keep heart alive...

	heart_ret = keep_heart(reader);
	if( 0 == heart_ret ){

            //心跳正常记录心跳发生时间
	    gettimeofday(&timeval, NULL);
	    last_heart_time = (unsigned long long)timeval.tv_sec;

	}else{

	        gettimeofday(&timeval, NULL);
	        cur_heart_time = (unsigned long long)timeval.tv_sec;
                //判断心跳是否超时(60秒)，超时则重连读写器
                if( (cur_heart_time - last_heart_time) >60 ){
                    //心跳超时处理
                  }

	}

	int i;
	//gpioVal[0]值为入口GPIO,默认GPIO8，gpioVal[1]值为入口GPIO,默认GPIO10
	int gpioVal[2]={0,0};

	for(i=0;i<gate_num;i++)
	{
	#if 0
		//获取入口车道地感状态
		get_gpio(GPIO8,&gpio8Val);

		//获取出口车道地感状态
		get_gpio(GPIO10,&gpio10Val);
	#endif
		get_gpio(gates[i].inductor_gpio,gpioVal[i]);


	}
        
        //如果任一车道地感有信号
	if( 1== gpioVal[0] || 1 == gpioVal[1] )
	{
		retVal = SIGNAL;	
		if( 1 == gpioVal[0] )
                      
                   //记录入口车道地感为有信号状态
		   whitchInduction[0] = 1;    
		   print_log(f_misc_running,"Enter GPIO=%d\n!!!",gpioVal[0]);

		if( 1 == gpioVal[1])
                   //记录出口车道地感为有信号状态
		   whitchInduction[1] = 2;//NO.2 induction have signal          
		   print_log(f_misc_running,"LEAVE GPIO=%d\n!!!",gpioVal[1]);

	}else{
		retVal = NOSIGNAL;
	}


	if( SIGNAL == retVal )
	{
	        gettimeofday(&timeval, NULL);
	        last_signal_time = (unsigned long long)timeval.tv_sec;
	}else if( NOSIGNAL == retVal )
	{
	        gettimeofday(&timeval, NULL);
	        cur_signal_time = (unsigned long long)timeval.tv_sec;

		if ( (cur_signal_time - last_signal_time) > inductor_signal_keep_time )
		{
                        //超过延读时间后清除入口车道地感状态
			whitchInduction[0]=0;

                        //超过延读时间后清除出口车道地感状态
			whitchInduction[1]=0;

		}else{//在延读时间范围内,地感视为有信号状态，并暂存相应车道地感状态
			//printf("open_time < inductor_signal_keep_time!!open_time=%d\n",open_time);
                        retVal = SIGNAL;
		}
	}
	return retVal;
}

void ThreadMonitorCapDeal(void)
{
	int retVal = 0;
        reader = NULL;

        //定义并初始化车道地感信号变量
	int whitchInduction[2] = {0,0};

	ConnectionReader();


	while(1)
	{
	       if(!strcmp(working_way,"INDUCTION"))
	       {
	       	   retVal = CheckLandInduction(whitchInduction);
	       }else{//working_way="POLL",系统采用轮询工作方式
		   
                   retVal = SIGNAL; 
	       }
                
		if( SIGNAL == retVal )
		{

			retVal = GetTagsAndDeal(whitchInduction);
			if( ERROR == retVal ){

			    ConnectionReader();

			}else if( SUCCESS == retVal ){

				if(!strcmp(working_way,"INDUCTION"))
			       {
			         usleep(get_tags_inteval*1000);
			       }else{//working_way="POLL",系统采用轮询工作方式
				   
			         usleep(get_tags_inteval*1000);
			       }		

			}

		}else if( NOSIGNAL == retVal ){
			usleep(200000);
			continue;
		}else if( ERROR == retVal ){
			ConnectionReader();
		}

	}//endof while(.....
}


/* --------------------------------------------------------------------------*/
/**
 * @功能  在历史车辆通信记录中寻找是否有指定车辆
 *
 * @参数 opt_info 查询的车辆信息
 *
 * @返回值   true 车辆在6s内数据已发现  false 车辆在6s内数据没有发现
 */
/* ----------------------------------------------------------------------------*/
bool FilterTags(STRUCT_OPERATE_INFO *opt_info)
{
	unsigned long long cur_time = 0;
	unsigned long long old_time = 0;
	int    i = 0;
	int    isFound = 0;
	int    isTimeError =0;

	struct timeval    timeval;
	gettimeofday(&timeval, NULL);
	cur_time = (unsigned long long)timeval.tv_sec * 1000000L + (unsigned long long)timeval.tv_usec;
	
	// 当前时间 为 当日自00:00:00 以来的秒数
	//cur_time = local->tm_hour * 3600 + local->tm_min *60 + local->tm_sec;

	for( i = 0;i<TAG_OLD_PASSED_MAX;i++ )
	{
		if ( g_old_passed_array[i].used==0 ) continue;
		if( (strncmp(opt_info->TID, g_old_passed_array[i].TID, TID_LEN)==0)
				&& (opt_info->ant_num == g_old_passed_array[i].ant_num) ) 
		{
			old_time = g_old_passed_array[i].time;
			if ( cur_time < old_time )// 时间回溯
			{
				isTimeError =1 ;
				break;
			}
			else
			{
				isFound = 1;
				break;
			}
		}
	}
	
	if ( isTimeError == 1 )// 时间回溯
	{
		memset(g_old_passed_array, 0x00, sizeof(OLDPASSEDOBJ)*TAG_OLD_PASSED_MAX);
		return true;
	}
	// 时间正常
	if( isFound == 1 )
	{
		old_time = g_old_passed_array[i].time;
		if( cur_time - old_time  > 6*1000000 )// 超过6s
		{
			g_old_passed_array[i].time = cur_time;
			return true;
		}
		else
		{
			return false;
		}
	}
	else//未找到
	{
		isFound = 0;// 是否有空间存放
		int    sel=-1; //选择的空间
		for(i=0;i<TAG_OLD_PASSED_MAX;i++)
		{
			if ( g_old_passed_array[i].used == 0 )
			{
				isFound=1;
				sel = i;
				break;
			}
		}
		if ( isFound == 0 )// 没有空间可用，清除过期数据
		{
			for(i=0;i<TAG_OLD_PASSED_MAX;i++)
			{
				if( cur_time - g_old_passed_array[i].time > 6*1000000 )
				{
					g_old_passed_array[i].used = 0;
					sel = i;
				}
			}
		}
		if ( sel == -1 )
		{
			// 概率很小
			return true;
		}
		else
		{
			g_old_passed_array[sel].used = 1;
			strcpy(g_old_passed_array[sel].TID, opt_info->TID);
                        g_old_passed_array[sel].ant_num = opt_info->ant_num;
			g_old_passed_array[sel].time = cur_time;
			return true;
		}
	}
	return false;
}

bool AddTagsToLedList(int operate_index)
{
	bool be_old = false;
	int j,n_gate_index;
	int cur_num;

	if( operate_index<0 || operate_index >= MAX_OPERATE_INFO_NUM){
		return false; 
	}

	n_gate_index = g_operate_info->operate_info[operate_index].gate_index;
	switch(n_gate_index)
	{
		case 0:
		case 1:
			break;
		default:
			return false;
	}

	cur_num = g_led_show_list[n_gate_index].led_info_cur_num;

	if(cur_num < LED_SHOW_MAX_NUM)
	{
		pthread_mutex_lock(&g_led_show_list[n_gate_index].mutex_led_show);

		for(j=0;j<cur_num;j++)
		{
			
			if(str_equal(g_operate_info->operate_info[operate_index].car_num,
				g_led_show_list[n_gate_index].led_show_array[j].car_num,CAR_NUM_LEN))
			{
				be_old = true;
				break;
			}
		}

		if(!be_old)
		{
			g_led_show_list[n_gate_index].led_show_array[cur_num].be_entry = 
				g_operate_info->operate_info[operate_index].be_enter;
			str_assign_value(g_operate_info->operate_info[operate_index].car_num, \
			g_led_show_list[n_gate_index].led_show_array[cur_num].car_num,CAR_NUM_LEN);

			/*
			   str_assign_value(gates[n_gate_index].led_ip, \
			   led_show_list_ptr[n_gate_index].led_show_oper[cur_num].led_ip,15);
				  
			   led_show_list_ptr[n_gate_index].led_show_oper[cur_num].led_port = gates[n_gate_index].led_port;
			*/

			g_led_show_list[n_gate_index].led_info_cur_num++;

  		}

		pthread_mutex_unlock(&g_led_show_list[n_gate_index].mutex_led_show);

  }//end for if(led_show_list_ptr.......

  return true;
}

bool OpenDoor(int operate_index,int openDoorMethodType,bool b_print_log)
{
	int gate_index = 0xffff;
	if( operate_index<0 || operate_index >= MAX_OPERATE_INFO_NUM ){
		return false; 
	}

	switch(openDoorMethodType){
		case RS485:
		case RELAY:
			break;
		default:
			return false;
	}

	switch(openDoorMethodType){
		case RS485:

			gate_index = g_operate_info->operate_info[operate_index].gate_index;
			com_open_roadblock(gates[gate_index].com_roadblock_fd,g_road_block_count);

			break;
		case RELAY:

			if( g_operate_info->operate_info[operate_index].ant_num == 1 ){
				set_gpio(12,1);
				usleep(100*1000);
				set_gpio(12,0);
			}else if( g_operate_info->operate_info[operate_index].ant_num == 2 ){
				set_gpio(14,1);
				usleep(100*1000);
				set_gpio(14,0);
			}//end for if( operate_info[.....

			// 组装struct SPassRecordLog
			STRUCT_OPERATE_INFO *op = &g_operate_info->operate_info[operate_index];
			struct SPassRecordLog   log;
			memset(&log, 0x00, sizeof(log));
			memcpy(log.m_Tid, op->TID, 16);
			time_t tt = time(NULL);
			struct tm *local = localtime(&tt);
			sprintf(log.m_Timestamp,"%04d/%02d/%02d %02d:%02d:%02d",  \
				local->tm_year+1900,local->tm_mon+1,local->tm_mday,local->tm_hour, \
					local->tm_min,local->tm_sec);
			log.m_Channel = op->gate_id;
			log.m_Direction = (op->io_type=='i')?0:1;
			log.m_PassResult = 1;
			
			// fill the TimeStamp
			unsigned long long nowtimestamp;
			struct timeval  nowtime;
			gettimeofday(&nowtime, NULL);
			nowtimestamp = (unsigned long long)nowtime.tv_sec * 1000000L
						+ (unsigned long long)nowtime.tv_usec;
			log.m_Meta.m_TimeStamp[0] =  (nowtimestamp>>32);
			log.m_Meta.m_TimeStamp[1] =  nowtimestamp & 0xFFFFFFFF;
			
			InsertPassRecordLog2(&log);// 发送数据

			
			break;
		default:
			return false;

	}//end for switch( openDoorMethodType....

	if( b_print_log ){

		if( 'i' == g_operate_info->operate_info[operate_index].io_type ){
			//print_log("TAG:%s Number:%s  Entered in!!!\n",g_operate_info->operate_info[operate_index].TID,operate_info[operate_index].car_num); 
			print_log(f_passed_success,"Number:%s Entered in!\n",g_operate_info->operate_info[operate_index].car_num); 
		}

		if( 'o' == g_operate_info->operate_info[operate_index].io_type ){
			//print_log("TAG:%s Number:%s  Leave away!!!\n",g_operate_info->operate_info[operate_index].TID,g_operate_info->operate_info[operate_index].car_num); 
			print_log(f_passed_success,"Number:%s Leave away!\n",g_operate_info->operate_info[operate_index].car_num); 
		}
		//   print_log("ant_num = %d\n",g_operate_info->operate_info[operate_index].ant_num); 

	}//end for if( b_print_log......

	return true;
}

/*
功能：将读到的标签进校验,该标签是否在配制文件中
参数：读到标签tid值 
返回值：匹配成功返回天线号，匹配失败返回-1
*/
int get_gate_index(int ant_id)
{
   int i,j,ant_num;
   if(ant_id > 4 || ant_id <= 0)
	return -1;
   for(i=0;i<gate_num;i++)
   {
	ant_num = gates[i].ant_num;
	for(j=0;i<ant_num;j++){
	  if(ant_id == gates[i].ants[j]) 
		return gates[i].ants[j];
	} 
   }

    return -1;
}

int GetTagsAndDeal(int *whitchInduction)
{
	int k,i;
	int retVal = 0;
	listnode node;
	int cards_num = 0;
	int n_gate_index = 0xffff;
	int induction = 0;
        struct   timeval    timeval;

	list result;
	result = list_new();

        //记录读标签操作当前时间，用于心跳判断条件
        gettimeofday(&timeval, NULL);
        last_heart_time = (unsigned long long)timeval.tv_sec;

	retVal = start_read_without_signal(reader,result);

	memset(g_tags_array,0,sizeof(TAGOBJ) * TAG_NUM_MAX);   
	g_tags_array_count = 0;

	if ( ERROR == retVal)
	{
		print_log(f_error,"start_read_without_signal() ERROR\n!");
		return ERROR;
	}


	FOR_EACH(node, result)
	{
	        struct tag* tag = (struct tag*)getdata(node);

		//判断标签数据有效性
		if( tag->antenna_id <0 || tag->antenna_id > 4 || 
				strlen(tag->tid)!=TID_LEN )
		{
			free(tag);
			continue;
		}

	/*  地感 对应天线的数据 */
  	       if( 1 == whitchInduction[0] && 2 == whitchInduction[1] ){
			//do nothing
		}else{
		
		  if( 1 == whitchInduction[0]){
			induction = whitchInduction[0]; 
		  }else if(2 == whitchInduction[1] ){
			induction = whitchInduction[1]; 
		  }
	
		  if(0 != induction){
			if(tag->antenna_id != induction){
				strcpy(tag->tid,"");
				free(tag);
				continue;
		        }
		   }
		
		}//end for if( 1 == whitchInduction[0...... 


		// 过滤标签
		int j;	
		int isFound=0;
		for( j=0; j<g_tags_array_count; j++)
		{
			if( g_tags_array[j].ant == tag->antenna_id && 
				strcmp(g_tags_array[j].tid, tag->tid )==0 )
			{
				g_tags_array[j].count++;
				isFound = 1;
			}
		}
		if( isFound == 0 )
		{
			strcpy(g_tags_array[g_tags_array_count].tid, tag->tid);
			g_tags_array[g_tags_array_count].ant = tag->antenna_id;
			g_tags_array[g_tags_array_count].count = 1;
			g_tags_array_count++;
			print_log(f_misc_running,"tag->antenna_id=%d\n!",tag->antenna_id);
			print_log(f_misc_running,"tag->tid=%s\n!",tag->tid);
		}
		free(tag);
	}

	list_delete(result);

	////////////////////////////以上是初步过滤标签完成////////////////////////////////////////////////// 

	if( g_tags_array_count >= TAG_NUM_MAX )
	{
		g_tags_array_count = TAG_NUM_MAX;
	}


	memset(g_operate_info->operate_info,0,MAX_OPERATE_INFO_NUM * sizeof(STRUCT_OPERATE_INFO));
	g_operate_info->cur_operate_num = 0;
	STRUCT_OPERATE_INFO      *operate=NULL;
	int                       opIndex=0;

	for( i=0;i<g_tags_array_count;i++ )
	{
	       char carnum[20] = {0};
	       int ret_cwl=0;

               ret_cwl =  CheckWhiteList(&g_tags_array[i].tid,carnum);

		if( -1 != ret_cwl )//找到
		{
			opIndex = g_operate_info->cur_operate_num;
			operate = &(g_operate_info->operate_info[opIndex]);
			//拷贝TID
			strcpy( operate->TID, g_tags_array[i].tid);
			//拷贝车牌号
			strcpy( operate->car_num, carnum);
			//拷贝天线号
			operate->ant_num = g_tags_array[i].ant;

			n_gate_index = get_gate_index(operate->ant_num);//获取门号
			if( -1 == n_gate_index )
			{
				print_log(f_error,"gate_index is error!\n");
				continue;
			}

			operate->gate_index = n_gate_index;
			operate->gate_id = gates[n_gate_index].gate_id;
			operate->io_type = gates[n_gate_index].gate_type;

			if ( operate->io_type == 'i' || operate->io_type == 'o' )
			{
				operate->be_enter = true;
			}
			g_operate_info->cur_operate_num++;

		}
		else// 未找到
		{
			print_log(f_passed_failed,"TID:%s is not in the whitelist!!!\n",g_tags_array[i].tid);
			continue;
		}


	}//end for for( i=0;i<g_tags_array_count;i++.....

	bool can_open = false;
	for( i=0;i<g_operate_info->cur_operate_num;i++)
	{
		can_open = FilterTags(&g_operate_info->operate_info[i]);
		if( can_open ){

			//add to Led List array to show 
			if( !AddTagsToLedList(i) ){
				print_log(f_error,"Error:AddTagsToLedList() faild!!\n");
			}

			if( !OpenDoor(i,RELAY,true) ){
				print_log(f_error,"Error:OpenDoor() faild!!\n");
			}//end for if( !OpenDoor(.....

		}//end for if( can_open )....
	}//end for for( i=0;i<cur_opera.....

	return SUCCESS;
}


 void ThreadLedShow(void* argument)
 {
         int i;
         int  arg = *(int*)argument;

         //only one led
         LEDSHOWINFO cp_one_led_show_oper;
         while (1)
         {
                 // copy from led_show_oper
                 pthread_mutex_lock(&g_led_show_list[arg].mutex_led_show);
                 if (g_led_show_list[arg].led_info_cur_num > 0)
                 {
                         memcpy(&cp_one_led_show_oper,&(g_led_show_list[arg]),sizeof(LEDSHOWINFO));

                         show_chepai(g_led_show_list->led_ip,g_led_show_list->led_port,cp_one_led_show_oper.car_num, \
                                         cp_one_led_show_oper.be_entry);

                         for (i = 0; i < g_led_show_list[arg].led_info_cur_num; i++)
                                 memcpy(&(g_led_show_list[arg].led_show_array[i]),&(g_led_show_list[arg].led_show_array[i+1]),sizeof(LEDSHOWINFO));

                         memset(&g_led_show_list[arg].led_show_array[g_led_show_list[arg].led_info_cur_num-1],0,sizeof(LEDSHOWINFO));
                         g_led_show_list[arg].led_info_cur_num -= 1;

                 }

                 pthread_mutex_unlock(&g_led_show_list[arg].mutex_led_show);

                 sleep (1);
         }
 }


//白名单请求线程
void ThreadRequestWhitelist(void)
{
        int ret;
	while(1)
	{
		sleep(request_whitelist_inteval);
		ret = client_recv_whitelist();
                if( 0 == ret )
                {
		        print_log(f_sync_whitelist,"update the white list successfully!!!");
                }else{
		        print_log(f_sync_whitelist,"update the white list failed!!!");
                }
	}
}

//断链续传线程函数
void ThreadResendPassrecord(void)
{
	int ret;
	int sended_count = 0;
	while(1)
	{
		sleep( passrecord_resend_inteval );

		//每次轮询，将最近passrecord_resend_loop_time小时的记录发送到服务
	        ret = ResendCachePassRecordLimitByDate( passrecord_resend_limit_time ,&sended_count);
		switch(ret)
		{
		     case PK_SUCCESS:
			//将发送成功的记录条数写入日志
			print_log(f_sended_server,"resend %d passrecord!!!",sended_count);
			break;
		     default:
		        //发送失败，将原因写入日志	
			print_log(f_sended_server,"error:resend passrecord failed %s",PKErrorMsg(ret));
			break;		
		}

	                
	}
}

// for kill fork
void ThreadKillFork()
{
	struct sigaction act;
	int sig = SIGTERM;
	pid_t pid = getpid();
	printf("main:waiting for sig kill forc pid = %d!\n",pid);
	sigemptyset(&act.sa_mask);
	act.sa_sigaction = before_fork_kill;
	act.sa_flags = SA_SIGINFO;
	if (sigaction(sig,&act,NULL)<0)
	{
		printf("install sigal error!\n");
	}
}

void before_fork_kill()
{
	if (reader != 0)
		close_reader(reader);
	print_log(f_sysinit,"succes close_reader before kill fork!");
	printf("fork be killed!close reader tcp!\n");

	if( NULL != f_sysinit )
		close_log_file(f_sysinit);

	if( NULL != f_misc_running )
		close_log_file(f_misc_running);

	if( NULL != f_error )
		close_log_file(f_error);

	if( NULL != f_passed_success )
		close_log_file(f_passed_success);

	if( NULL != f_passed_failed )
		close_log_file(f_passed_failed);

	if( NULL != f_sended_server )
		close_log_file(f_sended_server);

	if( NULL != f_sync_whitelist )
		close_log_file(f_sync_whitelist);

	exit(0);
}

