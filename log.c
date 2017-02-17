#include "log.h"
#include <unistd.h>
#define DEFAULT_LOG_SIZE 20*1024*1024
#define MAX_LOG_SIZE 50*1024*1024

extern int log_size;
pthread_mutex_t mtx_print_log = PTHREAD_MUTEX_INITIALIZER;
long filesize(FILE *stream)
{
   long curpos,length;
   curpos = ftell(stream);
   fseek(stream,0L,SEEK_END);
   length = ftell(stream);
   fseek(stream,curpos,SEEK_SET);
   return length;
}



FILE * open_log_file(char *path)
{
    FILE *file;
    if( NULL == path ){
	return NULL;
    }
    
    file = fopen(path,"a+");
    if( NULL == file ){
	fclose(file);
	return NULL;
    }

    return file;
}

void close_log_file(FILE *file)
{	
    if(NULL != file){
      fclose(file);
    }
}

void print_log(FILE *file,const char *ms, ... )  
{  
    long cur_file_size = 0;
    char wzLog[512] = {0};  
    char buffer[512] = {0};  

    unsigned char *path =  NULL;
    unsigned char *rawname = NULL;
    unsigned char *bakname = NULL;

    va_list args;  
    va_start(args, ms);  
    vsprintf( wzLog ,ms,args);  
    va_end(args);  
  
    if( NULL == file)
        return;
    pthread_mutex_lock(&mtx_print_log);
    time_t now;  
    time(&now);  
    struct tm *local;  
    local = localtime(&now);  

    //printf("%04d-%02d-%02d %02d:%02d:%02d %s\n", local->tm_year+1900, local->tm_mon+1,  
    //       local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec,  
    //       wzLog);  
       
    cur_file_size = filesize(file);

    if( log_size <= 0 || log_size > MAX_LOG_SIZE )
	log_size = DEFAULT_LOG_SIZE;	
       
    if( cur_file_size > log_size ){

        path = (unsigned char *)calloc(512,sizeof(unsigned char));
        usleep(5*1000);
        rawname = (unsigned char *)calloc(512,sizeof(unsigned char));
        usleep(5*1000);
        bakname = (unsigned char *)calloc(512,sizeof(unsigned char));
        usleep(5*1000);

        int fd = fileno(file);
        sprintf(path,"/proc/self/fd/%d",fd);
        memset(rawname,0,sizeof(rawname));
        readlink(path,rawname,sizeof(rawname)-1);

        close_log_file(file);
        file = NULL;
        strcpy(bakname,rawname);
        strcat(bakname,".bak");
        rename(rawname,bakname);
        usleep(5*1000);

       //create new log file	
       	file = open_log_file(rawname);
       	if( NULL != file ){
       	   printf("create log file!!\n");
       	}else{
       	   //do noting,still use the old file handle;
       	}

        //free temp buffers
        free(path);
        usleep(5*1000);

        free(rawname);
        usleep(5*1000);

        free(bakname);
        usleep(5*1000);
    }
     
    sprintf(buffer,"%04d-%02d-%02d %02d:%02d:%02d %s\n", local->tm_year+1900, local->tm_mon+1,  
                local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec,  
                wzLog);  
    fwrite(buffer,1,strlen(buffer),file);  
    fflush(file);
    pthread_mutex_unlock(&mtx_print_log);

    return ;  
}  

