#ifndef _LOG_INCLUDE
#define _LOG_INCLUDE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <syslog.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdbool.h>

FILE * open_log_file(char *path);
void close_log_file(FILE *file);
void print_log(FILE *file,const char *ms, ...);

#endif //_LOG_INCLUDE
