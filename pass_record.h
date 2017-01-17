#ifndef _SEND_RECORD_H
#define _SEND_RECORD_H
#include <stdio.h>
#include "common_sz.h"
	
//定义元数据
struct SLog
{
	unsigned int m_SeqNo[2];//序列号
	unsigned int m_TimeStamp[2];//时间戳
};

//定义通行记录结构
struct SPassRecordLog
{
	struct SLog  m_Meta;
	char         m_Tid[20];//tid
	char         m_Timestamp[20];
	int          m_Channel;
	int          m_Direction;//记录来车方向
	int          m_PassResult;//通行结果
	int          m_Flag;//用于记录发送状态，1为发送成功，0为发送失败
};

#endif

