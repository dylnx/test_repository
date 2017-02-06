#include "basic_func.h"

int file_read(char * path,char *buf)
{
    FILE *infile;
    infile = fopen(path,"rb");
    if (infile == NULL){
        printf("main:can`t open white list!\n");
        fclose(infile);
        return -1;
    }
    fseek(infile, 0, SEEK_END);
    int all_len = (int)ftell(infile);
    fseek(infile, 0, SEEK_SET);
    int read_len = 0,cur_len;
    while (all_len > 0){
        cur_len = fread(&buf[read_len],1,4096,infile);
        read_len += cur_len;
        all_len -= cur_len;
    }

    fclose(infile);
    return read_len;
}

bool readStringParam(char* buf,int buf_len, char* key,char* value)
{
    char line[400];
    int bufPos = 0;
    int linePos = 0;
    int valueIndex = 0;
    int valuePos = 0;
    while(bufPos<buf_len){
       if(buf[bufPos]=='\n' || (buf[bufPos]=='\r' && buf[bufPos+1]=='\n')){
           if(str_start_with(key,strlen(key),line,linePos))
	    {
              for(valueIndex=strlen(key)+1;valueIndex<linePos;valueIndex++){
                 value[valuePos++] = line[valueIndex];
              }
              value[valuePos] = '\0';
              return true;
           }
           if(buf[bufPos]=='\r' )
	   {
               bufPos= bufPos+2;
           }else{
               bufPos++;
           }
           memset(line,200,0);
           linePos = 0;
       }else{
          line[linePos++]=buf[bufPos++];
       }
    }

    if(str_start_with(key,strlen(key),line,linePos)){
        for(valueIndex=strlen(key);valueIndex<linePos;valueIndex++){
            value[valuePos++] = line[valueIndex];
        }
        value[valuePos] = '\0';
        return true;
    }

   return false;

}

bool readIntParam(char* buf,int buf_len, char* key,int* value)
{
    char result[1024];
    if(readStringParam(buf,buf_len,key,result)){
        *value = atoi(result);
        return true;
    }else{
        return false;
    }
}


bool get_config_info(char * buf,int len,char *sip,int *sport,char * rip,int *rport,int *rrate,char *path,int *c_id,int *g_num,STRUCT_GATE_INFO *g_info)
{
    int i = 0;
    int key_index = 0,gate_index = 0;
    for(i =0;i<len;i++)
    {
        if(buf[i] == ':')
        {
            key_index++;
            if (key_index == 1)
            {
                i++;
                int k = i;
                while (buf[i] != '/')
                {
                    if ((buf[i]<='9' || buf[i]>='0')||(buf[i] == '.'))
                        sip[i-k] = buf[i];
                    i++;
                }
                i++;
                *sport = atoi(&buf[i]);
                while(buf[i] != '\n' && buf[i] != '\r')
                    i++;
            }

            if (key_index == 2)
            {
                i++;
                int k = i;
                while (buf[i] != '/')
                {
                    if ((buf[i]<='9' || buf[i]>='0')||(buf[i] == '.'))
                        rip[i-k] = buf[i];
                    i++;
                }
                i++;
                *rport = atoi(&buf[i]);
				while (buf[i] != '/')
                        i++;
				i++;
//				*rrate = atoi(&buf[i]);
//				printf("reader_port:%d,reader_rate:%d!\n",*rport,*rrate);
                while(buf[i] != '\n' && buf[i] != '\r')
                    i++;
            }

            if (key_index == 3)
            {
                i++;
                int k = i;
                while (buf[i] != '/')
                {
                    path[i-k] = buf[i];
                    i++;
                }
                path[i-k+1] = '\0';

                i ++;
                *c_id = atoi(&buf[i]);
                i += 2;
            }

            if (key_index == 4)
            {
                i++;
                *g_num = atoi(&buf[i]);
		printf("gate_num:%d!\n",*g_num);
                while(buf[i] != '\n' && buf[i] != '\r')
                    i++;
            }
            if (key_index >= 5+gate_index*3)
            {
                int k = (key_index-5)/3;
                if ((key_index-5) % 3 == 0)
                {
                    int len = 0;
                    char c[60];
                    i += 1;
                    g_info[k].gate_type = buf[i];
                    i += 2;
                    g_info[k].gate_id= atoi(&buf[i]);
                    while (buf[i] != '/')
                        i++;

                    i++;
                    str_assign_value("/dev/",&(g_info[k].com_roadblock[0]),5);
                    str_get_sub_string(&buf[i],&(g_info[k].com_roadblock[5]),&len,'/');
                    i = i + len + 1;
                    if (g_info[k].gate_type != 'i')
                    {
                        continue;
                    }
                    i = i + len + 1;
                    str_get_sub_string(&buf[i],g_info[k].led_ip,&len,'/');
                    i = i + len + 1;
                    g_info[k].led_port = atoi(&buf[i]);
                    while (buf[i] != '/')
                        i++;
                    i++;
                    str_get_sub_string(&buf[i],g_info[k].gate_rights,&len,'\n');
                }
                if ((key_index-5) % 3 == 1)
                    g_info[k].ant_num = atoi(&buf[++i]);
                if ((key_index-5) % 3 == 2)
                {
                    int j = 0;
                    for (j=0;j<g_info[k].ant_num;j++)
                    {
                        g_info[k].ants[j] = atoi(&buf[++i]);
                        i++;
                    }
                    gate_index ++;
                }
            }
            if (key_index == 6)
            {

            }
         }
    }
    return true;
}

bool str_equal(unsigned char * str_1,unsigned char * str_2,int len)
{
    int i = 0;
    for(i=0; i < len; i++)
    {
        if (str_1[i] != str_2[i])
            return false;
    }
    return true;
}

bool str_start_with(unsigned char * substr,int substrlen,unsigned char * line,int linelen)
{
    int index = 0;
    while(index<linelen && index<substrlen){
       if(substr[index]!=line[index]){
            return false;
       }
       index++;
    }
    if(index==substrlen){
        return true;
    }else{
        return false;
    }

}

//int str_assign_value(unsigned char * str_from,unsigned char * str_to,int len)
int str_assign_value(char * str_from,char * str_to,int len)
{
    int i = 0,copy_len = 0;
    for(i = 0;i<len;i++)
    {
        if (str_from[i] == '\0')
            break;
        str_to[i] = str_from[i];
    }
    copy_len = i;
    for (;i<len;i++)
        str_to[i] = '\0';
    return copy_len;
}

bool str_get_sub_string(char * buf,char * des,int *len,char end_c)
{
    *len = 0;
    int i = 0;
    while (buf[i] != end_c)
    {
        if (*len > 60)
            return false;
        des[i] = buf[i];
        *len = *len +1;
        i++;
    }
    return true;
}



