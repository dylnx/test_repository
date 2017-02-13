#ifndef __ledapi__
#define __ledapi__

#include <stdbool.h>

int show_init(char *led_ip,int led_port,bool be_door);

int show_chepai(char *led_ip,int led_port,const char * pai,bool be_entry);

int show_camera(char *led_ip,int led_port,bool be_door,const char * pai,bool be_entry);

#endif


