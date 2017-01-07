#ifndef API_H_PVYP9UZN
#define API_H_PVYP9UZN
#include "common.h"
#include "linklist.h"
#define OK  0
#define ERROR -1
#define NOSIGNAL -2

#define MAX_ANTENNA 4

#define ANTENNA_1 0x0001
#define ANTENNA_2 0x0002
#define ANTENNA_3 0x0004
#define ANTENNA_4 0x0008
#define ANTENNA_NONE 0x0000

typedef enum
{
	PROTOCOL_HANGB             = 0x00,//行标
	PROTOCOL_GB                = 0x01,//国标
	PROTOCOL_ISO6C             = 0x02,//6C
	PROTOCOL_GJB               = 0x03,//军标
} AirProtocol;

typedef enum
{
	READMODE_HIGHSPEED         = 0x00,//高速
	READMODE_LOWSPEED          = 0x01,//低速
}ReadMode;

typedef enum{
	FREQ_FIXED                 = 0x00,//定频
	FREQ_DYN                   = 0x01,//跳频
} FreqMode;

typedef enum
{
	PARAM_PROTOCOL,//协议
    PARAM_READMODE, //读模式
	PARAM_FREQMODE, //调频设置
	PARAM_RF_ANTNUM, //天线号
	PARAM_RF_ANTPOWER,//功率
	PARAM_RF_ANTFRATE,//前向链路速率
	PARAM_RF_ANTRRATE,//反向链路速率
	PARAM_RF_ANTMODULATE, //前向链路调制方式
	PARAM_RF_ANTENCODE,//反向链路编码方式
	PARAM_DURTIME,//读命令持续的时间
} ReaderParam;


#define MAX_USER_SIZE 4
#define MAX_TID_LEN 100
#define MAX_EPC_LEN 100
#define MAX_DATA_LEN 100

struct tag{
	int reader_id;
	int antenna_id;
	u64 first_seen_time;
	u64 last_seen_time;
	u16 tag_count;
	int rssi;
	u16 priority;
	int ua_size;
	int write_status;
	u8 tid[MAX_TID_LEN];
	u8 epc[MAX_EPC_LEN];
	u8 data[MAX_USER_SIZE][MAX_DATA_LEN];
};


#define GB_TID 0x00
#define GB_EPC 0x10
#define GB_SEC 0x20
#define GB_USR 0x30

#define JB_TID 0x00
#define JB_EPC 0x10
#define JB_SEC 0x20
#define JB_USR 0x30

#define BB_TID 0x00
#define BB_EPC 0x10
#define BB_SEC 0x20
#define BB_USR 0x30

#define ISO6C_SEC 0x00
#define ISO6C_EPC 0x01
#define ISO6C_TID 0x02
#define ISO6C_USR 0x03

typedef struct Command{
	u8 area;
	u16 start_addr;
	u16 len;
	u32 token;
} Command;
typedef int reader_t;
typedef int (*callback)(struct tag*, void *result);

reader_t new_reader(const char *ip, int port);
int close_reader(reader_t r);
int init_reader(reader_t r);

int set_reader_param(reader_t r, int key, int value);
int set_read_command1(reader_t r, Command cmd);
int set_read_command2(reader_t r, Command cmd);
int set_write_command(reader_t r, Command cmd);

int start_read(reader_t reader, list result);
int start_read_without_signal(reader_t r, list result);
int start_write(reader_t r,void *data, list result);
int close_reader(reader_t r);
int start_read_with_callback(reader_t reader, callback result_handle, void *args);
int stop_read(reader_t reader);
int send_reboot1(reader_t reader);
int keep_hear(reader);

#endif /* end of include guard: API_H_PVYP9UZN */
