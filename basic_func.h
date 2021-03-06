#include "common_sz.h"

int file_read(char * path,char *buf);
bool get_config_info(char * buf,int len,char *sip,int *sport,char * rip,int *rport,int *rrate,char *path,int *c_id,int *g_num,STRUCT_GATE_INFO *g_info);

bool str_equal(unsigned char * str_1,unsigned char * str_2,int len);
bool str_start_with(unsigned char * substr,int substrlen,unsigned char * line,int linelen);

//int str_assign_value(unsigned char * str_from,unsigned char * str_to,int len);
int str_assign_value(char * str_from,char * str_to,int len);
bool str_get_sub_string(char * buf,char * des,int *len,char end_c);

