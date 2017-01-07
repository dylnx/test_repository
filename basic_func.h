#include "common_sz.h"

int file_read(char * path,char *buf);
bool get_config_info(char * buf,int len,char *sip,int *sport,char * rip,int *rport,int *rrate,char *path,int *c_id,int *g_num,STRUCT_GATE_INFO *g_info);
bool file_write_whitelist();
bool file_write_tempcarlist();

bool str_equal(unsigned char * str_1,unsigned char * str_2,int len);
bool str_start_with(unsigned char * substr,int substrlen,unsigned char * line,int linelen);

//int str_assign_value(unsigned char * str_from,unsigned char * str_to,int len);
int str_assign_value(char * str_from,char * str_to,int len);
bool str_get_sub_string(char * buf,char * des,int *len,char end_c);

bool get_seats_num(const char * str_seats_info,int len,int * p_seat);

int get_white_list_index(const char * cur_tid);
bool add_white_list(const char * cur_item,int len);
bool del_white_list(const char * cur_item,int len);
bool edit_white_list(const char * cur_item,int len);

