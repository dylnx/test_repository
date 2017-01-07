#include "operate_func.h"
#include "basic_func.h"
#include "common_sz.h"

bool check_right(char * rights,char * car_right)
{
    int i = 0;
    while(rights[i]<='9' && rights[i]>='0')
    {
        if (str_equal(car_right,&(rights[i]),2)){
            printf("check right:%s right be in!\n",car_right);
            return true;
        }
        i++;
    }
    printf("check right2:%s right error!\n",car_right);
    return false;
}

// white_list
/* --------------------------------------------------------------------------*/
/**
 * @功能  查询白名单
 *
 * @参数 tid  TID字符串
 *
 * @返回值 >=0 白名单索引  -1 未查找到 
 */
/* ----------------------------------------------------------------------------*/
int get_index_by_tid(char * tid)
{
    int index = -1,i = 0;
    for (i = 0;i < g_white_list->white_list_num; i++)
    {
        //if (str_equal(tid,g_white_list->white_list[i],TID_LEN))
        if (strncmp(tid,g_white_list->white_list[i],TID_LEN)==0)
        {
            index = i;
            break;
        }
    }
    return index;
}

//get gate index
int get_gate_index(int ant_id)
{
   if(ant_id > 4 || ant_id <= 0)
	return -1;
  //printf("gate0.ant=%d,gate1.ant=%d\n",gates[0].ants[0],gates[1].ants[0]);
   if(ant_id == gates[0].ants[0])
	return 0;

   if(ant_id == gates[1].ants[0])
	return 1;

    return -1;
}
