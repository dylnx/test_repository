#ifndef COMMON_SZ_DEFIN
#define COMMON_SZ_DEFIN

#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "log.h"
#include "pk_errno.h"

#define TID_LEN 16              
#define TAG_NUM_MAX  100
#define TAG_OLD_PASSED_MAX  1000 //max history passed list 
#define CAR_NUM_LEN 13          
#define CAR_RIGHT_LEN  2        
#define OPRATION_INFO_LEN 40 
#define MAX_OPERATE_INFO_NUM 100 
#define LED_SHOW_MAX_NUM 100     
#define TIME_STR_LEN 20

#define GPIO8 8
#define GPIO10 10
#define GPIO12 12
#define GPIO14 14

typedef struct struct_operate_info_s
{
    char TID[20];              
    char car_num[CAR_NUM_LEN];
    char card_type[CAR_RIGHT_LEN];
    char operate_time[TIME_STR_LEN]; 
    char ant_num;      
    int  gate_id;                   
    int  gate_index;                
    char io_type;                   
    bool be_enter;                  
}STRUCT_OPERATE_INFO;

typedef struct struct_get_tags_s
{
    char tid[20];
    char ant;
    int  count;
}TAGOBJ,*PTAGOBJ;

typedef struct struct_old_passed_obj_s
{
    char TID[20];
    int  ant_num;
    unsigned long long time;
    int  used;
}OLDPASSEDOBJ,*POLDPASSEDOBJ;

// per gate info
typedef struct gate_info_s
{
    char    gate_type;              // 'i' or 'o'
    char    gate_id;                // the id of gate defined by server
    int     inductor_gpio;
    int     relay_gpio;
    char    com_roadblock[20];      // the com port for linking roadblock
    int     com_roadblock_fd ;      // the handler of operate com port 
    char    led_ip[20];             // the ip of led
    int     led_port;               // the port of led
    char    gate_rights[20];        // the rights which can entry
    char    ant_num;                // the number of ants using for this gate
    char    ants[8];                // the id of per ant
}STRUCT_GATE_INFO;

typedef enum openDoorMethodType
{
    RS485,
    RELAY 
}OPENDOORMETHODTYPE;

typedef enum statType
{
    SUCCESS,
    ERROR,
    SIGNAL,
    NOSIGNAL,
    TAGNOTFOUNT
}STATTYPE;

//LED显示信息
struct led_send_info
{
    char    ip[20];
    char    car_num[20];
    int     port;
    bool    be_entry;
};


typedef struct operate_info_s
{
    pthread_mutex_t mutex_operate_info;
    STRUCT_OPERATE_INFO operate_info[MAX_OPERATE_INFO_NUM];
    int cur_operate_num;
}OPERATEINFO,*POPERATEINFO;
POPERATEINFO g_operate_info;



int reader;

//for filter
PTAGOBJ g_tags_array;
int     g_tags_array_count;
POLDPASSEDOBJ g_old_passed_array;



//用于保存从config.ini读取的配制数据
char server_ip[15];
char reader_ip[15];
int server_port;
int reader_port;
int reader_rate;
int request_whitelist_inteval;
int inductor_signal_keep_time;
int passrecord_resend_inteval;
int passrecord_resend_limit_time;
int dev_id;
char exist_led[10];
int g_road_block_count;
int gate_num;
char working_way[20];
int  get_tags_inteval;
char door_open_method[20];

int log_size; //defult value

int g_ants_element;

//gate info
STRUCT_GATE_INFO gates[2];

//log file pointer
FILE *f_sysinit;
FILE *f_misc_running;
FILE *f_error;
FILE *f_passed_success;
FILE *f_passed_failed;
FILE *f_sended_server;
FILE *f_sync_whitelist;


#endif

