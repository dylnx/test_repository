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

#define TID_LEN 16              // TID\u5b57\u8282\u6570
#define TAG_NUM_MAX  100
#define TAG_OLD_PASSED_MAX  1000 //max history passed list 
#define MAX_WHITELIST_NUM 3000  // \u767d\u540d\u5355\u6700\u5927\u6570\u91cf
#define MAX_TEMPCARLIST_NUM 500
#define CAR_NUM_LEN 13          // \u8f66\u724c\u53f7\u5b57\u8282\u6570
#define CAR_RIGHT_LEN  2        // \u8f66\u5361\u901a\u884c\u7c7b\u522b\u6807\u8bc6\u7b26\u7684\u5b57\u8282\u6570
#define OPRATION_INFO_LEN 40    // \u6bcf\u6761\u4e1a\u52a1\u8bb0\u5f55\u5b57\u8282\u6570
#define TIME_STR_LEN 20         // \u65f6\u95f4\u5b57\u7b26\u4e32\u5b57\u8282\u6570
#define POSITION_LEN 2          // \u5b57\u7b26\u4e32\u5b57\u8282\u6570
#define MAX_UNDEAL_CARD_NUM 10  //
#define MAX_OPERATE_INFO_NUM 10 //
#define MAX_SEND_INFO_NUM 10   //
#define LED_SHOW_MAX_NUM 10     //

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
    char ant_num;      //by myjalo
    int  gate_id;                   
    int  gate_index;                
    char io_type;                   
    bool be_enter;                  
}STRUCT_OPERATE_INFO;

typedef struct struct_get_tags_s
{
    char tid[20];
    char ant_num;
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
    char    com_roadblock[20];      // the com port for linking roadblock
    int     com_roadblock_fd ;      // the handler of operate com port 
    char    led_ip[15];             // the ip of led
    int     led_port;               // the port of led
    char    gate_rights[20];        // the rights which can entry
    char    ant_num;                // the number of ants using for this gate
    char    ants[4];                // the id of per ant
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

// for led show
typedef struct led_show_info_s
{
    char    car_num[CAR_NUM_LEN];
    bool    be_entry;
}LEDSHOWINFO;

typedef struct led_show_info_list_s
{
    pthread_mutex_t mutex_led_show;
    LEDSHOWINFO led_show_array[LED_SHOW_MAX_NUM];
    char   led_ip[15];
    int    led_port;
    int    led_info_cur_num;
}LED_SHOW_LIST,*PLED_SHOW_LIST;


typedef struct white_list_s
{
    pthread_mutex_t mutex_white_list;
    unsigned char white_list[MAX_WHITELIST_NUM][TID_LEN+2+CAR_NUM_LEN];
    int white_list_num;
}WHITELIST,*PWHITELIST;
PWHITELIST g_white_list;

typedef struct operate_info_s
{
    pthread_mutex_t mutex_operate_info;
    STRUCT_OPERATE_INFO operate_info[MAX_OPERATE_INFO_NUM];
    int cur_operate_num;
}OPERATEINFO,*POPERATEINFO;
POPERATEINFO g_operate_info;

typedef struct send_info_s
{
    pthread_mutex_t mutex_send_info;
    STRUCT_OPERATE_INFO send_info[MAX_SEND_INFO_NUM];
    int cur_send_info_num;
    unsigned int read_pos;
    unsigned int write_pos;
}SENDINFO,*PSENDINFO;
PSENDINFO g_send_info;

typedef struct show_camera_s
{
    pthread_mutex_t mutex_led_show_camera;
    LEDSHOWINFO led_show_camera[LED_SHOW_MAX_NUM];
    char   led_ip[15];
    int    led_port;
    int led_info_camera_num;
}SHOWCAMERA,*PSHOWCAMERA;
PSHOWCAMERA g_show_camera;

int reader;

//for filter
PTAGOBJ g_tags_array;
int     g_tags_array_count;
char *g_tag_flag;
POLDPASSEDOBJ g_old_passed_array;
//int           g_old_passed_array_count;
PLED_SHOW_LIST g_led_show_list;


//config desc
char server_ip[15];
char reader_ip[15];
int server_port;
int reader_port;
int reader_rate;
int white_list_path[100];
int jointcompute_id;
int time_of_update_list;
int inductor_signal_keep_time;
int log_file_size; //defult value

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

