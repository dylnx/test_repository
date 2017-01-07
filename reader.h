#ifndef READER_H_WKPVMJYQ
#define READER_H_WKPVMJYQ

#include "api.h"
#include "tcpcommon.h"


struct AntennaConfig
{
	int power;//功率
	int frate;//前向链路速率
	int brate;//反向链路速率
	int modulate;//调制方式
	int codec;//编码方式
};

struct ReaderConfig
{
	int id;
	char ip[50];
	int port;
	int type;
	int durtime;//读命令持续的时间  (ms)
	int antenna[MAX_ANTENNA];
	int workmode;
	int protocol;
	int frequence;
	struct AntennaConfig antenna_config[MAX_ANTENNA];
};

typedef struct Reader
{
	struct ReaderConfig config;
	MConn session;
	int enabled;
	int open_switch;
	int is_working; //读写器是否正在工作
	Mbyte start_cmd;
	Mbyte stop_cmd;
	Mbyte access_cmd;
	Command read_cmd1;
	Command read_cmd2;
	int cmd1_type;
	int cmd2_type;
	Command write_cmd;
	void *inner_data;
} Reader;

struct ReaderEvent{
	int type;
	int value1;
	int value2;
	char message[100];
};

reader_t new_reader(const char *ip, int port);

int set_reader_param(reader_t r, int key, int value);
int set_read_command1(reader_t r, Command cmd);
int set_read_command2(reader_t r, Command cmd);
int set_write_command(reader_t r, Command cmd);

int start_read(reader_t reader, list result);
int start_read_with_callback(reader_t reader, callback result_handle, void *args);
int stop_read(reader_t reader);
int send_reboot(reader_t r);
int send_reboot1(reader_t r);


#endif /* end of include guard: READER_H_WKPVMJYQ */
