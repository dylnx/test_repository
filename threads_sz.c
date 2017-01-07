#include "threads_sz.h"
#include "serial.h"
#include "operate_func.h"
#include "api.h"
#include "ledapi.h"
#include "gpio.h"
#include "serial.h"
#include<unistd.h>     
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>      
#include<termios.h>    
#include<errno.h>      


#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define OK  0
#define ERROR -1
#define NOSIGNAL -2
#define ANTNUMERROR -3

list result;

void before_fork_kill();

extern reader_t * reader = NULL;

long int newest_open_time = 0;
long int last_heart_time = 0;
unsigned int total_count=0;
long int flag_count =0;

int ConnectionReader();
int CheckLandInduction(int *whitchInduction);
int GetTagsAndDeal(int *whitchInduction);

/***************ConnectionReader*****************
FunctionName:ConnectionReader
Parameter:Null
Author:myjalo
FunctionDescription:

 *************************************************/
int ConnectionReader()
{
	int retVal=0;
	time_t tt = 0;
	struct tm *local = NULL;

	while(1)
	{
		close_reader(reader);		
		usleep(100);

		reader = new_reader(reader_ip,reader_port);

		if(reader!=NULL){
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
				print_log(f_sysinit,"connected reader successfully!!!");
				sleep(4);
				tt = time(NULL);
				local = localtime(&tt);
				last_heart_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;
				break;
			}
		}
		else{
			print_log(f_sysinit,"new_reader() NULL\n!");
			usleep(500*1000);
			continue;
		}
	} // end of while(.....

	return OK;
}

int CheckLandInduction(int *whitchInduction)
{
	int heart_ret=0,retVal=0;
	time_t newest_tt = 0;
	struct tm *newest_local = NULL;
	long int cur_time = 0;
	unsigned int gpio8Val,gpio10Val;

	gpio8Val = 0;
	gpio10Val= 0;

	//keep heart alive...
	heart_ret = keep_heart(reader);
	if( 0 == heart_ret ){
		newest_tt = time(NULL);
		newest_local = localtime(&newest_tt);
		last_heart_time = newest_local->tm_hour * 3600 + newest_local->tm_min * 60 + newest_local->tm_sec;
	}else{
		newest_tt = time(NULL);
		newest_local = localtime(&newest_tt);
		cur_time = newest_local->tm_hour * 3600 + newest_local->tm_min * 60 + newest_local->tm_sec;
		if ( cur_time < last_heart_time )
			cur_time += 24 * 3600;
		int open_time = cur_time - last_heart_time;
		if( open_time > 45){
			// ConnectionReader();
		}

	}
	get_gpio(GPIO8,&gpio8Val);
	get_gpio(GPIO10,&gpio10Val);

	if( 1== gpio8Val|| 1 == gpio10Val )
	{
		retVal = OK;	
		if( 1 == gpio8Val )
			whitchInduction[0] = 1;//NO.1 induction have signal          
		print_log(f_misc_running,"gpio8Val=%d\n!!!",gpio8Val);

		if( 1 == gpio10Val)
			whitchInduction[1] = 2;//NO.2 induction have signal          
		print_log(f_misc_running,"gpio10Val=%d\n!!!",gpio10Val);

	}else{
		retVal = NOSIGNAL;
	}


	if( OK == retVal )
	{
		// have signal, no error; may be 0 or 100 card,
		newest_tt = time(NULL);
		newest_local = localtime(&newest_tt);
		newest_open_time = newest_local->tm_hour * 3600 + newest_local->tm_min * 60 + newest_local->tm_sec;
	}else if( NOSIGNAL == retVal )
	{
		newest_tt = time(NULL);
		newest_local = localtime(&newest_tt);
		cur_time = newest_local->tm_hour * 3600 + newest_local->tm_min * 60 + newest_local->tm_sec;
		if (cur_time < newest_open_time)
			cur_time += 24 * 3600;
		int open_time = cur_time - newest_open_time;


		if ( open_time > inductor_signal_keep_time )
		{
			pthread_mutex_lock(&g_operate_info->mutex_operate_info);
			memset(g_operate_info->operate_info,0,MAX_OPERATE_INFO_NUM * sizeof(STRUCT_OPERATE_INFO));
			g_operate_info->cur_operate_num = 0;
			pthread_mutex_unlock(&g_operate_info->mutex_operate_info);

			//print_log("TIME OUT!!!CheckLandInduction() open_time > inductor_signal_keep_time!!\n!!!");
			//printf("TIME OUT!!!CheckLandInduction() open_time > inductor_signal_keep_time!!\n!!!");

			//after inductior_signal_keep_time seconds,let inductions reset to no singl status for next.
			//printf("before 30 seconds, whitchInduction[0]=%d,whitchInduction[1]=%d\n",whitchInduction[0],whitchInduction[1]);
			whitchInduction[0]=0;
			whitchInduction[1]=0;
			//printf("after 30 seconds, whitchInduction[0]=%d,whitchInduction[1]=%d\n",whitchInduction[0],whitchInduction[1]);

			return NOSIGNAL;
		}else{
			//printf("open_time < inductor_signal_keep_time!!open_time=%d\n",open_time);
			return OK;
		}
	}
	return OK;
}

void ThreadMonitorCapDeal(void)
{
	int retVal = 0;
	int whitchInduction[2] = {0,0};

	ConnectionReader();
	while(1)
	{
		retVal = CheckLandInduction(whitchInduction);
		if( OK == retVal )
		{

			retVal = GetTagsAndDeal(whitchInduction);
			if( ERROR == retVal ){

				//send_reboot1(reader);
				ConnectionReader(reader);

			}else if( OK == retVal ){

				usleep(200*1000);

			}else if( ANTNUMERROR == retVal ){

				continue;
			}
		}else if( NOSIGNAL == retVal ){

			//NO SIGNAL
			usleep(200000);
			continue;
		}else if( ERROR == retVal ){
			ConnectionReader(reader);
		}
	}//endof while(.....
}

int  GetRawArrayFlag(TAGOBJ rawTags[],int rawLen,char *flag)
{
	int i,j;
	for( i=0; i<rawLen; i++ )
	{
		for( j=i+1; j<rawLen; j++ )
		{
			if(0 == flag[j])
				continue;
			if( !strncmp(g_tags_array[i].tid,g_tags_array[j].tid,TID_LEN) ){
				flag[j]=0;
			}else{
				flag[j]=1;
			}
		}
	}//end for for( i=0...

	/*
	   for(i=0;i<rawLen;i++)
	   {
	   printf("+g_tag_flag[%d]=%d\n",i,flag[i]);
	   }
	   */
	return 0;
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
//bool FilterTags(STRUCT_OPERATE_INFO *opt_info,POLDPASSEDOBJ _old_passed_array)
bool FilterTags(STRUCT_OPERATE_INFO *opt_info)
{
	unsigned long long cur_time = 0;
	unsigned long long old_time = 0;
	int    i = 0;
	int    isFound = 0;
	int    isTimeError =0;
//	time_t tt = 0;
//	struct tm *local = NULL;

	//tt = time(NULL);
	//local = localtime(&tt);
	struct timeval    timeval;
	gettimeofday(&timeval, NULL);
	cur_time = (unsigned long long)timeval.tv_sec*1000000 + (unsigned long long)timeval.tv_usec;
	
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
	//memset(g_operate_info->operate_info,0,MAX_OPERATE_INFO_NUM * sizeof(STRUCT_OPERATE_INFO));

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
	if(g_led_show_list[n_gate_index].led_info_cur_num < LED_SHOW_MAX_NUM)
	{
		pthread_mutex_lock(&g_led_show_list[n_gate_index].mutex_led_show);

		for(j=0;j<g_led_show_list[n_gate_index].led_info_cur_num;j++)
		{
			if(str_equal(g_operate_info->operate_info[operate_index].car_num,g_led_show_list[n_gate_index].led_show_array[j].car_num,CAR_NUM_LEN))
			{
				be_old = true;
				break;
			}
		}

		if(!be_old)
		{
			g_led_show_list[n_gate_index].led_show_array[g_led_show_list[n_gate_index].led_info_cur_num].be_entry = \
																													g_operate_info->operate_info[operate_index].be_enter;
			str_assign_value(g_operate_info->operate_info[operate_index].car_num, \
					g_led_show_list[n_gate_index].led_show_array[g_led_show_list[n_gate_index].led_info_cur_num].car_num, \
					CAR_NUM_LEN);
			/*
			   str_assign_value(gates[n_gate_index].led_ip, \
			   led_show_list_ptr[n_gate_index].led_show_oper[led_show_list_ptr[n_gate_index].led_info_cur_num].led_ip, \
			   15);

			   led_show_list_ptr[n_gate_index].led_show_oper[led_show_list_ptr[n_gate_index].led_info_cur_num].led_port = gates[n_gate_index].led_port;
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
			com_open_roadblock(gates[gate_index].com_roadblock_fd,gate_index);

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


			g_send_info->send_info[g_send_info->write_pos].gate_id  = g_operate_info->operate_info[operate_index].gate_id;
			g_send_info->send_info[g_send_info->write_pos].be_enter = g_operate_info->operate_info[operate_index].be_enter;
			memcpy(g_send_info->send_info[g_send_info->write_pos].TID,g_operate_info->operate_info[operate_index].TID,TID_LEN);


			time_t tt = time(NULL);
			struct tm *local = localtime(&tt);
			sprintf(g_send_info->send_info[g_send_info->write_pos].operate_time,"%04d/%02d/%02d %02d:%02d:%02d",  \
					local->tm_year+1900,local->tm_mon+1,local->tm_mday,local->tm_hour, \
					local->tm_min,local->tm_sec);

			g_send_info->write_pos++;
			g_send_info->write_pos %= MAX_SEND_INFO_NUM;

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

int GetTagsAndDeal(int *whitchInduction)
{
	int k,i;
	int retVal = 0;
	listnode node;
	int cards_num = 0;
	int raw_cnt;
	int n_gate_index = 0xffff;
	int induction = 0;

	list result;
	result = list_new();
	retVal = start_read_without_signal(reader,result);


	memset(g_tags_array,0,sizeof(TAGOBJ) * TAG_NUM_MAX);   
	g_tags_array_count = 0;
	memset(g_tag_flag,1,sizeof(char) * TAG_NUM_MAX);   
	if (retVal == ERROR)
	{
		print_log(f_error,"start_read_without_signal() ERROR\n!");
		return ERROR;
	}

	i=0; 
	FOR_EACH(node, result)
	{
		struct tag* tag = (struct tag*)getdata(node);
		/*  地感 对应天线的数据 */
		//		if( 1 == whitchInduction[0] && 2 == whitchInduction[1] ){
		//			//do nothing
		//		}else{
		//
		//			if( 1 == whitchInduction[0]){
		//				induction = whitchInduction[0]; 
		//			}else if(2 == whitchInduction[1] ){
		//				induction = whitchInduction[1]; 
		//			}
		//
		//			if(0 != induction){
		//
		//				if(tag->antenna_id != induction){
		//					strcpy(tag->tid,"");
		//					free(tag);
		//					continue;
		//				}
		//
		//			}
		//
		//		}//end for if( 1 == whitchInduction[0...... 

		//判断标签数据有效性
		if( tag->antenna_id <0 || tag->antenna_id > 4 || 
				strlen(tag->tid)!=TID_LEN )
		{
			free(tag);
			continue;
		}


		//if(strcmp(tag->tid,""))
		//{
		//valid the ant
		//	switch(tag->antenna_id)
		//	{
		//		case 1:
		//		case 2:
		//		case 3:
		//		case 4:
		//			break;
		//		default:
		//			print_log(f_misc_running,"ant id is error!\n");
		//			free(tag);
		//			continue;                    
		//	}

		// 过滤标签
		int j;	
		int isFound=0;
		for( j=0; j<g_tags_array_count; j++)
		{
			if( g_tags_array[j].ant_num == tag->antenna_id && 
					strcmp(g_tags_array[j].tid, tag->tid )==0 )
			{
				g_tags_array[j].count++;
				isFound = 1;
			}
		}
		if( isFound == 0 )
		{
			strcpy(g_tags_array[i].tid, tag->tid);
			g_tags_array[i].ant_num = tag->antenna_id;
			g_tags_array[i].count = 1;
			g_tags_array_count++;
			print_log(f_misc_running,"tag->antenna_id=%d\n!",tag->antenna_id);
			print_log(f_misc_running,"tag->tid=%s\n!",tag->tid);
		}
		i++;	
		free(tag);
	}

	list_delete(result);
	//	raw_cnt = g_tags_array_count;
	////////////////////////////////////////////////////////////////////////////////// 

	//printf("raw_cnt = %d\n",raw_cnt);	
	if( g_tags_array_count >= TAG_NUM_MAX )
	{
		g_tags_array_count = TAG_NUM_MAX;
	}

	//the first filter 
	//	GetRawArrayFlag(g_tags_array,raw_cnt,g_tag_flag);


	memset(g_operate_info->operate_info,0,MAX_OPERATE_INFO_NUM * sizeof(STRUCT_OPERATE_INFO));
	g_operate_info->cur_operate_num = 0;
	STRUCT_OPERATE_INFO      *operate=NULL;
	int                       opIndex=0;

	for( i=0;i<g_tags_array_count;i++ )
	{
		//printf("g_tag_flag[%d]=%d\n",i,g_tag_flag[i]);
		//	if( g_tag_flag[i] == 1 )
		//		if( g_tag_array[i].count  )
		//		{

		pthread_mutex_lock(&g_white_list->mutex_white_list);
		//int k = get_index_by_tid(g_operate_info->operate_info[g_operate_info->cur_operate_num].TID);
		int k = get_index_by_tid(g_tags_array[i].tid);
		pthread_mutex_unlock(&g_white_list->mutex_white_list);

		if(k != -1)//找到
		{
			opIndex = g_operate_info->cur_operate_num;
			operate = &(g_operate_info->operate_info[opIndex]);

			strcpy( operate->TID, g_tags_array[i].tid);
			operate->ant_num = g_tags_array[i].ant_num;
			n_gate_index = get_gate_index(operate->ant_num);// * zhanghong 获取门号 不确定 *//

			//str_assign_value(g_tags_array[i].tid,g_operate_info->operate_info[g_operate_info->cur_operate_num].TID,TID_LEN);   
			//g_operate_info->operate_info[g_operate_info->cur_operate_num].ant_num = g_tags_array[i].ant_num;    
			//n_gate_index = get_gate_index(g_operate_info->operate_info[g_operate_info->cur_operate_num].ant_num);
			if( -1 == n_gate_index )
			{
				print_log(f_error,"gate_index is error!\n");
				continue;
			}
			operate->gate_index = n_gate_index;
			operate->gate_id = gates[n_gate_index].gate_id;
			operate->io_type = gates[n_gate_index].gate_type;

			//g_operate_info->operate_info[g_operate_info->cur_operate_num].gate_index = n_gate_index;
			//g_operate_info->operate_info[g_operate_info->cur_operate_num].gate_id = gates[n_gate_index].gate_id;
			//g_operate_info->operate_info[g_operate_info->cur_operate_num].io_type = gates[n_gate_index].gate_type;

			pthread_mutex_lock(&g_white_list->mutex_white_list);
			char * p = g_white_list->white_list[k];
			strcpy(operate->car_num, p+TID_LEN+2);  
			strncpy(operate->card_type, p+TID_LEN, 2);
			//				str_assign_value(g_white_list->white_list[k]+TID_LEN+2,g_operate_info->operate_info[g_operate_info->cur_operate_num].car_num,CAR_NUM_LEN);
			//				str_assign_value(g_white_list->white_list[k]+TID_LEN,g_operate_info->operate_info[g_operate_info->cur_operate_num].card_type,2);
			pthread_mutex_unlock(&g_white_list->mutex_white_list);

			//				if(g_operate_info->operate_info[g_operate_info->cur_operate_num].io_type == 'i'){
			//					g_operate_info->operate_info[g_operate_info->cur_operate_num].be_enter = true;
			//				}else if(g_operate_info->operate_info[g_operate_info->cur_operate_num].io_type == 'o'){
			//					g_operate_info->operate_info[g_operate_info->cur_operate_num].be_enter = true;
			//				}
			if ( operate->io_type == 'i' || operate->io_type == 'o' )
			{
				operate->be_enter = true;
			}
			g_operate_info->cur_operate_num++;


		}
		else// 未找到
		{
			print_log(f_passed_failed,"TID:%s is not in the whitelist!!!\n",g_tags_array[i].tid);
			return OK;
		}


		//}//end for if( g_tag_flag[i] == 1..... 

	}//end for for( i=0;i<g_tags_array_count;i++.....


	//printf("cur_operate_num = %d\n",g_operate_info->cur_operate_num);         


	bool can_open = false;
	for( i=0;i<g_operate_info->cur_operate_num;i++)
	{

		//bool FilterTags(STRUCT_OPERATE_INFO *opt_info,POLDPASSEDOBJ _old_passed_array)
		can_open = FilterTags(&g_operate_info->operate_info[i]);// g_old_passed_array
		if( can_open ){

			//add to Led List array to show 
			if( !AddTagsToLedList(i) ){
				print_log(f_error,"Error:AddTagsToLedList() faild!!\n");
			}
			//bool OpenDoor(int operate_index,OPENDOORMETHODTYPE openDoorMethodType,bool b_print_log)
			if( !OpenDoor(i,RELAY,true) ){
				print_log(f_error,"Error:OpenDoor() faild!!\n");
			}//end for if( !OpenDoor(.....

		}//end for if( can_open )....
	}//end for for( i=0;i<cur_opera.....


	return OK;
	}


#if 1
	void ThreadLedShow(void* argument)
	{
		int i;
		int  arg = *(int*)argument;



#if 1
		// init led
		//g_led_show_list[arg].led_info_cur_num = 0;
		//memset(g_led_show_list[arg].led_show_array,0,LED_SHOW_MAX_NUM * sizeof(LEDSHOWINFO));

		// only one led
		LEDSHOWINFO cp_one_led_show_oper;
		while (1)
		{
			/////////////////////////////// show chepai ///////////////////////////////

			// copy from led_show_oper
			pthread_mutex_lock(&g_led_show_list[arg].mutex_led_show);
			if (g_led_show_list[arg].led_info_cur_num > 0)
			{
				memcpy(&cp_one_led_show_oper,&(g_led_show_list[arg].led_show_array[0]),sizeof(LEDSHOWINFO));

				show_chepai(g_led_show_list->led_ip,g_led_show_list->led_port,cp_one_led_show_oper.car_num, \
						cp_one_led_show_oper.be_entry,false);

				for (i = 0; i < g_led_show_list[arg].led_info_cur_num; i++)
					memcpy(&(g_led_show_list[arg].led_show_array[i]),&(g_led_show_list[arg].led_show_array[i+1]),sizeof(LEDSHOWINFO));

				memset(&g_led_show_list[arg].led_show_array[g_led_show_list[arg].led_info_cur_num-1],0,sizeof(LEDSHOWINFO));
				g_led_show_list[arg].led_info_cur_num -= 1;

			}

			pthread_mutex_unlock(&g_led_show_list[arg].mutex_led_show);

			sleep (1);
		}
#endif
	}
#endif


	void ThreadInfoToServer(void)
	{
		while (1)
		{
			sleep(3);
			client_send_operinfo();
		}
	}

	void ThreadConnectWhitelist(void)
	{
		// extern int time_of_update_list;
		while (1)
		{
			sleep(time_of_update_list);
			client_recv_whitelist();
			// client_recv_tempcarlist();
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

	void ThreadCamera()
	{
		extern int jointcompute_id;
		if (jointcompute_id != 11)
			return;

		long int time_begin = 0, time_current = 0;
		time_t newest_tt = time(NULL);
		struct tm *newest_local = localtime(&newest_tt);
		newest_tt = time(NULL);
		newest_local = localtime(&newest_tt);
		time_begin = newest_local->tm_hour * 3600 + newest_local->tm_min * 60 + newest_local->tm_sec;
		time_current = time_begin;

		bool bRet = false;
		int sock = -1;
		while(1)
		{
			newest_tt = time(NULL);
			newest_local = localtime(&newest_tt);
			if(sock == -1)
			{
				sock = client_init_socket();
				continue;
			}
			bRet = client_camera(sock);
			if (bRet)
				time_begin = newest_local->tm_hour * 3600 + newest_local->tm_min * 60 + newest_local->tm_sec;
			else
			{
				time_current = newest_local->tm_hour * 3600 + newest_local->tm_min * 60 + newest_local->tm_sec;
				if (time_current < time_begin)
					time_current += 24 * 3600;
				if (time_current - time_begin > 20)
				{
					close(sock);
					sock = -1;
				}
			}
		}
	}
