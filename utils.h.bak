#ifndef UTILS_H
#define UTILS_H

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include "err.h"

typedef int (*funp)(void* args);

char* PRINTTIME(struct timeval timeinfo);
size_t strtrim(char *str);
int adv_atoi(char* str, int base);
void create_daemon_process(funp main,void *args);
int	create_pidfile(const char *file);
char *gettimestr(u64 time);
char *getbytestr(Mbyte byte);

extern int is_debug;

#define DEBUG(...)\
    do{\
        if( is_debug == 1){\
            char debug_time[40] = {0};\
            struct timeval detail_time;\
            gettimeofday(&detail_time, NULL);\
            time_t rawtime;\
            struct tm *timeinfo;\
            time(&rawtime);\
            timeinfo = localtime(&(detail_time.tv_sec));\
            sprintf(debug_time,"%4d-%02d-%02d %02d:%02d:%02d.%06d",\
                  timeinfo->tm_year+1900,\
                  timeinfo->tm_mon+1,\
                  timeinfo->tm_mday,\
                  timeinfo->tm_hour,\
                  timeinfo->tm_min,\
                  timeinfo->tm_sec,\
                  *(int *)&detail_time.tv_usec);\
            fprintf(stderr, "%s [%s][%s][%d]: ",\
                    debug_time, __FUNCTION__, __FILE__, __LINE__);\
            fprintf(stderr, __VA_ARGS__);\
            fprintf(stderr, "\n");\
            fflush(stderr);\
        }\
    }\
    while(0);

	// This macro call the m_init();
#define USING(m)\
	({\
	 int ret = 0;\
	 if (mod_using(m##_init,#m) != 0) {\
	 ret = -1;\
	 }\
	 ret;\
	 })
// this macro used by USING(m)
#define mod_using(init,m) \
	({\
	 int ret = 0;\
	 if (init() != SUCCESS) {\
	 ret = -1;\
	 }\
	 ret;\
	 })

#define malloc_z(type) \
	({type* ptr = (type*)malloc(sizeof(type));\
	assert(ptr);\
	memset(ptr, 0, sizeof(type));\
	ptr;})


# if __BYTE_ORDER == __LITTLE_ENDIAN
#define ntoh64(x) bswap64(x)
#define hton64(x) bswap64(x)
# else
#define ntoh64(x) (x)
#define hton64(x) (x)
# endif

# define bswap64(x) \
	((((x) & 0xFF00000000000000ull) >> 56)\
	 | (((x) & 0x00FF000000000000ull) >> 40)\
	 | (((x) & 0x0000FF0000000000ull) >> 24)\
	 | (((x) & 0x000000FF00000000ull) >> 8 )\
	 | (((x) & 0x00000000FF000000ull) << 8 )\
	 | (((x) & 0x0000000000FF0000ull) << 24)\
	 | (((x) & 0x000000000000FF00ull) << 40)\
	 | (((x) & 0x00000000000000FFull) << 56))

#define SLEEP(x) \
	do{\
		typeof (x)  _x = (x);\
		while(_x-- >0){\
			printf(".");fflush(stdout);\
			sleep(1);\
		}\
		printf("\n");\
	}while(0)
#endif
