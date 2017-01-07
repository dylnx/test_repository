#ifndef __ledapi__
#define __ledapi__
#include <stdbool.h>
int show_init(char *led_ip,int led_port,bool be_door);
//int show_seats(char *led_ip,int led_port,bool be_door,const char *seat_info);
int show_chepai(char *led_ip,int led_port,const char * pai,bool be_entry,bool be_clear);
int show_camera(char *led_ip,int led_port,bool be_door,const char * pai,bool be_entry);
//int show_chepai(const char * pai);

//int show_chepai1(const char *pai,int allowed);

//int show_chepai2(const char *pai,int direction,int allowed);

#endif


