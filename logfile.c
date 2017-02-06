#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "bigint.h"
#include "cond_thread.h"
#include "pass_record.h"
#include "logfile.h"

#define RESEND_RECORD_NUM 9999 

struct LogFile g_PassRecordLogInfo;
extern struct CondThread *g_PassRecordLogWriteFileThread;
extern struct CondThread *g_PassRecordSendThread;

static	struct SPassRecordLog *fakedata;

/*
功能：
	初始化日志文件
参数：
返回：
	-1/文件错误；0/失败；1/成功
*/
int InitLogFile(struct LogFile *logFile)
{
	FILE		*fp;
	unsigned int	ii, mm, nn;
	unsigned int	cnt;
	struct SLog  	*pLog;
	unsigned int	minSeq[2];
	unsigned int	maxSeq[2];
	unsigned int	minStamp[2];
	unsigned int	maxStamp[2];
	unsigned int	first, last;
	unsigned char  	*pLogBuffer;
    	unsigned int    LogBufferLen;
	
	LogBufferLen = logFile->m_Meta.m_RecordSize * logFile->m_Meta.m_BufRecord;
	
	pLogBuffer = malloc(LogBufferLen);
	if( pLogBuffer==NULL )
	{
		return -1;
	}

	//申请用于发送断链续传通行记录缓冲区
        *fakedata = (struct SPassRecordLog*)calloc(1,sizeof(struct SPassRecordLog)*RESEND_RECORD_NUM);
	if( NULL == fakedata )
	{
		printf("fakedata is NULL!!!\n");
		return -1;	
	}

	maxSeq[0]=0;
	maxSeq[1]=0;
	minSeq[0]=0xFFFFFFFF;
	minSeq[1]=0xFFFFFFFF;
	mm=0;
	cnt=0;
	first=0;
	last=0;

	//测试日志文件是否存在
	fp=fopen(logFile->m_Meta.m_FileName, "rb");

	//如果文件不存在
	if ( fp==NULL )
	{
		fp=fopen(logFile->m_Meta.m_FileName, "wb");
		if ( fp==NULL )
		{
			free(pLogBuffer);
			printf("can not create %s log file\n", logFile->m_Meta.m_FileName);
			logFile->m_Meta.m_Fp = NULL;
			return -1;
		}

		memset(pLogBuffer, 0x00, LogBufferLen);
		for ( ii=0;   ii<logFile->m_Meta.m_MaxRecord;    ii+=logFile->m_Meta.m_BufRecord )
		{
			nn=fwrite(pLogBuffer, logFile->m_Meta.m_RecordSize, logFile->m_Meta.m_BufRecord, fp);
			if ( nn!=logFile->m_Meta.m_BufRecord )
				break;
			mm += nn;
		}
		fclose(fp);

		if ( mm<logFile->m_Meta.m_MinRecord )
		{
			free(pLogBuffer);
			printf("%s log file too small\n", logFile->m_Meta.m_FileName);
			logFile->m_Meta.m_Fp=NULL;
			return -1;
		}
		logFile->m_CurrentCnt = 0;
		logFile->m_TotalCnt = mm;
		logFile->m_First = 0;
		logFile->m_Last = 0;
		logFile->m_maxSeq[0] = 0;
		logFile->m_maxSeq[1] = 0;
		logFile->m_minSeq[0] = 0;
		logFile->m_minSeq[1] = 0;
		logFile->m_maxStamp[0] = 0;
		logFile->m_maxStamp[1] = 0;
		logFile->m_minStamp[0] = 0;
		logFile->m_minStamp[1] = 0;
	}
	//如果文件存在，扫描文件
	else
	{
		while ( 1==1 )
		{
			//从日志文件读取多个记录
			nn=fread(pLogBuffer, logFile->m_Meta.m_RecordSize, logFile->m_Meta.m_BufRecord, fp);
			//文件读完
			if ( nn<1 )
				break;

			//检查各个记录
			for ( ii=0;    ii<nn;    ii++ )
			{
				pLog= (struct SLog*)(pLogBuffer+ii*logFile->m_Meta.m_RecordSize);

				//空白记录
				if ( (pLog->m_SeqNo[0]==0) && (pLog->m_SeqNo[1]==0) )
					continue;

				//寻找序号最小的记录位置
				if ( Compare64(pLog->m_SeqNo, minSeq)<0 )
				{
					first = mm+ii; 
					minSeq[0]=pLog->m_SeqNo[0];
					minSeq[1]=pLog->m_SeqNo[1];
					minStamp[0]=pLog->m_TimeStamp[0];
					minStamp[1]=pLog->m_TimeStamp[1];
				}
				//寻找序号最大的记录位置
				if ( Compare64(pLog->m_SeqNo, maxSeq)>0 )
				{
					last = mm+ii; 
					maxSeq[0]=pLog->m_SeqNo[0];
					maxSeq[1]=pLog->m_SeqNo[1];
					maxStamp[0]=pLog->m_TimeStamp[0];
					maxStamp[1]=pLog->m_TimeStamp[1];
				}
				cnt++;
			}

			mm += nn;
			if ( nn!=logFile->m_Meta.m_BufRecord )
				break;
		}
		fclose(fp);

		if ( mm<logFile->m_Meta.m_MinRecord )
		{
			free(pLogBuffer);
			printf("log file too small\n");
			logFile->m_Meta.m_Fp=NULL;
			return -1;
		}

		//现在，first,last,cnt,mm
		logFile->m_CurrentCnt = cnt;
		logFile->m_TotalCnt = mm;
		logFile->m_First = first;
		logFile->m_Last = last;
		logFile->m_maxSeq[0] = maxSeq[0];
		logFile->m_maxSeq[1] = maxSeq[1];
		logFile->m_minSeq[0] = minSeq[0];
		logFile->m_minSeq[1] = minSeq[1];
		logFile->m_maxStamp[0] = maxStamp[0];
		logFile->m_maxStamp[1] = maxStamp[1];
		logFile->m_minStamp[0] = minStamp[0];
		logFile->m_minStamp[1] = minStamp[1];
	}

	//重新打开日志文件
	fp =fopen(logFile->m_Meta.m_FileName, "r+b");
	if ( fp==NULL )
	{
		free(pLogBuffer);
		printf("can not open log file\n");
		logFile->m_Meta.m_Fp=NULL;
		return -1;
	}
	logFile->m_Meta.m_Fp = fp;
	pthread_mutex_init(&logFile->m_Meta.m_lock, NULL);
	free(pLogBuffer);
	return 1;
	
}

/*
功能：
	将一条日志写入日志文件
	此函数应该互斥调用
参数：
	pLog：
返回：
	-1/文件错误；0/失败；1/成功
*/
int InsertLogFile(struct LogFile *logFile, struct SLog *pLog)
{
	if ( logFile == NULL || pLog == NULL ) return 0;
	struct SLog         sLog;
	long				offset;
	int					ret;

	if ( logFile->m_Meta.m_Fp ==NULL )
		return -1;

	//计算记录的序号

	//序号回卷，可能引起问题
	if ( 
		(logFile->m_maxSeq[0]==0xFFFFFFFF)
		&&
		(logFile->m_maxSeq[1]==0xFFFFFFFF)
	)
	{
		//64比特的序号用完，应该将日志文件全部复位为0
		//实际不会出现
		logFile->m_maxSeq[0]=0;
		logFile->m_maxSeq[1]=1;
	}
	else if ( logFile->m_maxSeq[1]==0xFFFFFFFF )
	{
		logFile->m_maxSeq[0]++;
		logFile->m_maxSeq[1]=0;
	}
	else 
	{
		logFile->m_maxSeq[1]++;
	}

	pLog->m_SeqNo[0]=logFile->m_maxSeq[0];
	pLog->m_SeqNo[1]=logFile->m_maxSeq[1];

	//日志文件没有记录
	if ( logFile->m_CurrentCnt==0 )
	{
		//记录数目
		logFile->m_CurrentCnt++;

		logFile->m_minSeq[0] = pLog->m_SeqNo[0];
		logFile->m_minSeq[1] = pLog->m_SeqNo[1];

		logFile->m_minStamp[0] = pLog->m_TimeStamp[0];
		logFile->m_minStamp[1] = pLog->m_TimeStamp[1];

		logFile->m_maxStamp[0] = pLog->m_TimeStamp[0];
		logFile->m_maxStamp[1] = pLog->m_TimeStamp[1];

		//这时logFile->m_First==logFile->m_Last;
		//记录写入m_Last位置
	}
	//日志文件已满
	else if ( logFile->m_CurrentCnt==logFile->m_TotalCnt )
	{
		//丢去最老的记录
		logFile->m_First++;
		if ( logFile->m_First>=logFile->m_TotalCnt )
			logFile->m_First -= logFile->m_TotalCnt;
		//计算第一条记录的偏移
		offset = logFile->m_First * logFile->m_Meta.m_RecordSize;
		ret = fseek(logFile->m_Meta.m_Fp, offset, SEEK_SET);
		if ( ret!=0 )
		{
			return 0;
		}
		//读取文件
		ret = fread(&sLog, sizeof(struct SLog), 1, logFile->m_Meta.m_Fp);
		if ( ret!=1 )		
		{
			return -1;
		}

		logFile->m_minSeq[0] = sLog.m_SeqNo[0];
		logFile->m_minSeq[1] = sLog.m_SeqNo[1];

		logFile->m_minStamp[0] = sLog.m_TimeStamp[0];
		logFile->m_minStamp[1] = sLog.m_TimeStamp[1];

		//记录写入m_Last位置
//		logFile->m_Last = (logFile->m_Last+1) % logFile->m_TotalCnt;
		logFile->m_Last++;
		if ( logFile->m_Last>=logFile->m_TotalCnt )
			logFile->m_Last -= logFile->m_TotalCnt;
	}
	else
	{
		logFile->m_maxStamp[0] = pLog->m_TimeStamp[0];
		logFile->m_maxStamp[1] = pLog->m_TimeStamp[1];

		//记录数目
		logFile->m_CurrentCnt++;
		//记录写入m_Last位置
//		logFile->m_Last = (logFile->m_Last+1) % logFile->m_TotalCnt;
		logFile->m_Last++;
		if ( logFile->m_Last>=logFile->m_TotalCnt )
			logFile->m_Last -= logFile->m_TotalCnt;
	}

	//计算偏移
	offset = logFile->m_Last * logFile->m_Meta.m_RecordSize;
	ret = fseek(logFile->m_Meta.m_Fp, offset, SEEK_SET);
	if ( ret!=0 )
	{
		return -1;
	}
	//写入文件
	ret = fwrite(pLog, logFile->m_Meta.m_RecordSize, 1, logFile->m_Meta.m_Fp);
	if ( ret!=1 )		
	{
		return -1;
	}
	//写入磁盘
	ret = fflush(logFile->m_Meta.m_Fp);
	if ( ret!=0 )		
	{
		return -1;
	}

	return 1;
}

/*
功能：
	根据日志记录序号，读取一条日志
	此函数应该互斥调用
参数：
	seqNo：指定的序号
	pLog：返回的记录
	minNo：返回的现有最小的序号
	maxNo：返回的现有最大的序号
返回：
	-1/文件错误；0/没有指定的记录；1/成功
*/
int GetLogBySeqNo(struct LogFile *logFile, unsigned int seqNo[], struct SLog *pLog, 
	unsigned int minNo[], unsigned int maxNo[])
{
	long				offset;
	int					ret;
	unsigned int		delta[2], pos;
	
	if ( logFile==NULL || logFile->m_Meta.m_Fp == NULL )
		return -1;
	//日志文件没有记录
	if ( logFile->m_CurrentCnt==0 )
		return 0;

	minNo[0] = logFile->m_minSeq[0];
	minNo[1] = logFile->m_minSeq[1];
	maxNo[0] = logFile->m_maxSeq[0];
	maxNo[1] = logFile->m_maxSeq[1];

	//如果指定的序号小于第一条记录的序号
	if ( Compare64(seqNo, logFile->m_minSeq)<0 )
		return 0;
	//如果指定的序号大于最后一条记录的序号
	if ( Compare64(seqNo, logFile->m_maxSeq)>0 )
		return 0;

	//计算相对位置
	ret = Substract64(seqNo, logFile->m_minSeq, delta);
	//减法错误
	if ( ret!=0 ) 
		return 0;
	//序号相差太大
	if ( delta[0]!=0 ) 
		return 0;
	if ( delta[1]>logFile->m_TotalCnt )
		return 0;

	//计算指定的记录的实际位置
	pos = logFile->m_First + delta[1];
	if ( pos>=logFile->m_TotalCnt )
		pos -= logFile->m_TotalCnt;

	//计算指定的记录的偏移
	offset = pos * logFile->m_Meta.m_RecordSize;
	ret = fseek(logFile->m_Meta.m_Fp, offset, SEEK_SET);
	if ( ret!=0 )
		return -1;

	//读取文件
	ret = fread(pLog, logFile->m_Meta.m_RecordSize, 1, logFile->m_Meta.m_Fp);
	if ( ret!=1 )		
		return -1;

	return 1;
}
/*
功能：
	根据日志记录序号，更新一条日志
	此函数应该互斥调用
参数：
	seqNo：指定的序号
	pLog：指定的新记录
	minNo：返回的现有最小的序号
	maxNo：返回的现有最大的序号
返回：
	-1/文件错误；0/没有指定的记录；1/成功
*/
int SetLogBySeqNo(struct LogFile *logFile, unsigned int seqNo[], struct SLog *pLog, unsigned int minNo[], unsigned int maxNo[])
{
	long				offset;
	int					ret;
	unsigned int		delta[2], pos;
	
	if ( logFile==NULL || logFile->m_Meta.m_Fp == NULL )
		return -1;
	//日志文件没有记录
	if ( logFile->m_CurrentCnt==0 )
		return 0;

	minNo[0] = logFile->m_minSeq[0];
	minNo[1] = logFile->m_minSeq[1];
	maxNo[0] = logFile->m_maxSeq[0];
	maxNo[1] = logFile->m_maxSeq[1];

	//如果指定的序号小于第一条记录的序号
	if ( Compare64(seqNo, logFile->m_minSeq)<0 )
		return 0;
	//如果指定的序号大于最后一条记录的序号
	if ( Compare64(seqNo, logFile->m_maxSeq)>0 )
		return 0;

	//计算相对位置
	ret = Substract64(seqNo, logFile->m_minSeq, delta);
	//减法错误
	if ( ret!=0 ) 
		return 0;
	//序号相差太大
	if ( delta[0]!=0 ) 
		return 0;
	if ( delta[1]>logFile->m_TotalCnt )
		return 0;

	//计算指定的记录的实际位置
	pos = logFile->m_First + delta[1];
	if ( pos>=logFile->m_TotalCnt )
		pos -= logFile->m_TotalCnt;

	//计算指定的记录的偏移
	offset = pos * logFile->m_Meta.m_RecordSize;
	ret = fseek(logFile->m_Meta.m_Fp, offset, SEEK_SET);
	if ( ret!=0 )
		return -1;

	//读取文件
	ret = fwrite(pLog, logFile->m_Meta.m_RecordSize, 1, logFile->m_Meta.m_Fp);
	if ( ret!=1 )		
		return -1;

	return 1;
}

/*
功能：
	根据标签时间戳，读取一条日志(读取第一套时间戳匹配的记录)
	此函数应该互斥调用
参数：
	timeStamp：指定的时间戳
	pLog：返回的记录
	loc: 记录位置
	minStamp：返回的现有最小的时间戳
	maxStamp：返回的现有最大的时间戳
返回：
	-1/文件错误；0/没有指定的记录；1/成功；
	2/这时，pLog是第一个大于指定时间戳的记录
*/
int GetLogByStamp(struct LogFile *logFile, unsigned int timeStamp[], struct SLog *pLog, 
	unsigned int minStamp[], unsigned int maxStamp[])
{
	struct SLog		    *sLog = pLog;
	long				offset;
	int					ret, flag=0;
	unsigned int		pos;
	unsigned int		low, high, mid;
	
	if ( logFile==NULL || logFile->m_Meta.m_Fp == NULL )
		return -1;

	//日志文件没有记录
	if ( logFile->m_CurrentCnt==0 )
		return 0;

	minStamp[0] = logFile->m_minStamp[0];
	minStamp[1] = logFile->m_minStamp[1];
	maxStamp[0] = logFile->m_maxStamp[0];
	maxStamp[1] = logFile->m_maxStamp[1];

	//如果指定的时间戳小于第一条记录的时间戳
	if ( Compare64(timeStamp, logFile->m_minStamp)<0 )
		return 0;
	//如果指定的时间戳大于最后一条记录的时间戳
	if ( Compare64(timeStamp, logFile->m_maxStamp)>0 )
		return 0;

	low = logFile->m_First;
	high =  logFile->m_Last;
	if ( high<low )
		high += logFile->m_TotalCnt;
	while ( high>=low )
	{
		mid = (low + high) >> 1;
		pos = mid;
		if ( pos>=logFile->m_TotalCnt )
			pos -= logFile->m_TotalCnt;

		//计算指定的记录的偏移
		offset = pos * logFile->m_Meta.m_RecordSize;
		ret = fseek(logFile->m_Meta.m_Fp, offset, SEEK_SET);
		if ( ret!=0 )
			return -1;

		//读取文件
		ret = fread(sLog, logFile->m_Meta.m_RecordSize, 1, logFile->m_Meta.m_Fp);
		if ( ret!=1 )		
			return -1;

		ret= Compare64(sLog->m_TimeStamp, timeStamp);
		if ( ret==0 )
		{
			flag=0;
			break;
		}
		else if ( ret<0 )
		{
			flag = -1;
			if ( mid==0xFFFFFFFF )
				break;
			low = mid + 1;
		}
		else
		{
			flag = 1;
			if ( mid==0 )
				break;
			high = mid - 1;
		}
	}
	//memcpy(pLog, sLog, logFile->m_Meta.m_RecordSize);
	if ( flag!=0 )
	{
		//这时，pLog是第一个大于指定时间戳的记录
		return 2;
	}

	//正常退出。[mid]是需要的位置，由于时间戳可能重复，
	//向前查找
	while ( mid>low )
	{
		mid--;
		pos = mid;
		if ( pos>=logFile->m_TotalCnt )
			pos -= logFile->m_TotalCnt;

		//计算指定的记录的偏移
		offset = pos * logFile->m_Meta.m_RecordSize;
		ret = fseek(logFile->m_Meta.m_Fp, offset, SEEK_SET);
		if ( ret!=0 )
			return -1;

		//读取文件
		ret = fread(sLog, logFile->m_Meta.m_RecordSize, 1, logFile->m_Meta.m_Fp);
		if ( ret!=1 )		
			return -1;

		ret= Compare64(sLog->m_TimeStamp, timeStamp);
		if ( ret!=0 )
		{
//			memcpy(pLog, sLog, logFile->m_Meta.m_RecordSize);
			break;
		}
	}
	return 1;

}

/*
功能：
	根据清除Log的内容
	重置g_*LogInfo 的标志
	清空文件
参数：
返回：
	0/失败；1/成功；
*/
int ClearLog(struct LogFile *logFile)
{
	int                             ii;
	int                             ret;
	if ( logFile==NULL || logFile->m_Meta.m_Fp == NULL )
		return -1;
	logFile->m_CurrentCnt = 0;
	logFile->m_TotalCnt = 0;
	logFile->m_First = 0;
	logFile->m_Last = 0;
	logFile->m_maxSeq[0] = 0;
	logFile->m_maxSeq[1] = 0;
	logFile->m_minSeq[0] = 0;
	logFile->m_minSeq[1] = 0;
	logFile->m_maxStamp[0] = 0;
	logFile->m_maxStamp[1] = 0;
	logFile->m_minStamp[0] = 0;
	logFile->m_minStamp[1] = 0;
	char *pLogBuffer = (char*)malloc(logFile->m_Meta.m_RecordSize*logFile->m_Meta.m_BufRecord);
	memset(pLogBuffer, 0x00, logFile->m_Meta.m_RecordSize*logFile->m_Meta.m_BufRecord);
	ret = fseek(logFile->m_Meta.m_Fp, 0, SEEK_SET);
	if ( ret != 0 ) {
		free(pLogBuffer);
		return 0;
	}
	for ( ii=0;   ii<(int)logFile->m_Meta.m_MaxRecord;    ii+=logFile->m_Meta.m_BufRecord )
	{
		ret=fwrite(pLogBuffer, logFile->m_Meta.m_RecordSize, logFile->m_Meta.m_BufRecord, logFile->m_Meta.m_Fp);
		if ( (unsigned int)ret!=logFile->m_Meta.m_BufRecord )
		{
			free(pLogBuffer);
			return 0;
		}
	}
	free(pLogBuffer);
	return 1;
}


/////////////////////////////

/*
功能：
	初始化通行记录日志文件
参数：
返回：
	-1/文件错误；0/失败；1/成功
*/
int InitPassRecordLogFile()
{
	strcpy( g_PassRecordLogInfo.m_Meta.m_FileName, RECORD_FILE_NAME);
	g_PassRecordLogInfo.m_Meta.m_MaxRecord = MAX_RECORD_LOG_FILE_REC;
	g_PassRecordLogInfo.m_Meta.m_MinRecord = MIN_RECORD_LOG_FILE_REC;
	g_PassRecordLogInfo.m_Meta.m_BufRecord = MAX_RECORD_LOG_BUF;
	g_PassRecordLogInfo.m_Meta.m_RecordSize = sizeof(struct SPassRecordLog);
	return InitLogFile(&g_PassRecordLogInfo);
}

/*
功能：
	根据清除Log的内容
	重置g_*LogInfo 的标志
	清空文件
参数：
返回：
	0/失败；1/成功；
*/
int ClearPassRecordLog()
{
	return ClearLog(&g_PassRecordLogInfo);
}

////////////////////////////

/*
功能：
	插入一条通行记录日志到内存标签日志队列
	会触发写车牌日志文件
参数：
	pLicLog：车牌记录
返回：
*/
void InsertPassRecordLog1(struct SPassRecordLog *pPassRecordLog)
{
	struct SPassRecordLog		*temp;
	
	temp = (struct SPassRecordLog *)malloc(sizeof(struct SPassRecordLog));
	memcpy(temp, pPassRecordLog, sizeof(struct SPassRecordLog));
	// 触发读标签
	TriggerThread(g_PassRecordLogWriteFileThread, 0, temp);
}



/*
功能：
	写通行记录日志线程处理函数
参数：
	pPassRecordLog：车牌记录
返回：
*/
void PassRecordLogHandle(void *args)
{
	if ( args == NULL ) return ;
	struct _cond_thread_msg *msg = args;
	struct SPassRecordLog *temp = msg->data;
	InsertPassRecordLogFile(temp);
	free(temp);
}

/*
功能：
	插入一条通行记录日志到内存标签日志队列
	会触发发送通信记录
参数：
	pPassRecrodLog：通行记录
返回：
*/
void InsertPassRecordLog2(struct SPassRecordLog *pPassRecordLog)
{
	struct SPassRecordLog		*temp;
	
	temp = (struct SPassRecordLog *)malloc(sizeof(struct SPassRecordLog));
	memcpy(temp, pPassRecordLog, sizeof(struct SPassRecordLog));
	// 触发读标签
	TriggerThread(g_PassRecordSendThread, 0, temp);
}



/*
功能：
	写通行记录日志线程处理函数
参数：
	pPassRecordLog：车牌记录
返回：
*/
void PassRecordSendHandle(void *args)
{
	if ( args == NULL ) return ;
	struct _cond_thread_msg *msg = args;
	struct SPassRecordLog *temp = msg->data;
	// send the result
	printf("The PassRecord is sending.... [%d]:%s\n",
			temp->m_Channel, temp->m_Tid);
	// TODO
	int ret = send_pass_record(temp, 0);
	if( ret == 0 )
	{
		temp->m_Flag = 0;
	}
	else
	{
		temp->m_Flag = 1;
	}
	InsertPassRecordLog1(temp);
	free(temp);
}

/*
功能：
	将一条通行记录写入日志文件
	此函数应该互斥调用
参数：
	pPassRecordLog：
返回：
	-1/文件错误；0/失败；1/成功
*/
int InsertPassRecordLogFile(struct SPassRecordLog *pPassRecordLog)
{
	int ret = 0;
	pthread_mutex_lock(&g_PassRecordLogInfo.m_Meta.m_lock);
	ret = InsertLogFile(&g_PassRecordLogInfo, (struct SLog*)pPassRecordLog);
	pthread_mutex_unlock(&g_PassRecordLogInfo.m_Meta.m_lock);
	return ret;
}

/*
功能：
	根据日志记录序号，读取一条日志
	此函数应该互斥调用
参数：
	seqNo：指定的序号
	pPassRecordLog：返回的记录
	minNo：返回的现有最小的序号
	maxNo：返回的现有最大的序号
返回：
	-1/文件错误；0/没有指定的记录；1/成功
*/
int GetPassRecordLogBySeqNo(unsigned int seqNo[], struct SPassRecordLog *pPassRecordLog, 
	unsigned int minNo[], unsigned int maxNo[])
{
	int ret = 0;
	pthread_mutex_lock(&g_PassRecordLogInfo.m_Meta.m_lock);
	ret = GetLogBySeqNo(&g_PassRecordLogInfo, seqNo, (struct SLog *)pPassRecordLog, minNo, maxNo); 
	pthread_mutex_unlock(&g_PassRecordLogInfo.m_Meta.m_lock);
	return ret;
}
/*
功能：
	根据日志记录序号，写入一条日志
	此函数应该互斥调用
参数：
	seqNo：指定的序号
	pPassRecordLog：返回的记录
	minNo：返回的现有最小的序号
	maxNo：返回的现有最大的序号
返回：
	-1/文件错误；0/没有指定的记录；1/成功
*/
int SetPassRecordLogBySeqNo(unsigned int seqNo[], struct SPassRecordLog *pPassRecordLog, 
	unsigned int minNo[], unsigned int maxNo[])
{
	int ret = 0;
	pthread_mutex_lock(&g_PassRecordLogInfo.m_Meta.m_lock);
	ret = SetLogBySeqNo(&g_PassRecordLogInfo, seqNo, (struct SLog *)pPassRecordLog, minNo, maxNo); 
	pthread_mutex_unlock(&g_PassRecordLogInfo.m_Meta.m_lock);
	return ret;
}
/*
功能：
	写入一条日志, 覆盖原有的记录
	此函数应该互斥调用
参数：
	pPassRecordLog：记录
返回：
	-1/文件错误；0/没有指定的记录；1/成功
*/
int WriteBackPassRecordLog(struct SPassRecordLog *pPassRecordLog)
{
	int ret = 0;
	unsigned int seqNo[2];
	unsigned int minNo[2];
	unsigned int maxNo[2];

	if( pPassRecordLog==NULL ) return 0;
	seqNo[0] = pPassRecordLog->m_Meta.m_SeqNo[0];
	seqNo[1] = pPassRecordLog->m_Meta.m_SeqNo[1];
	ret = SetPassRecordLogBySeqNo(seqNo, pPassRecordLog, minNo, maxNo);
	return ret;
}

/*
功能：
	根据标签时间戳，读取一条日志(读取第一套时间戳匹配的记录)
	此函数应该互斥调用
参数：
	timeStamp：指定的时间戳
	pPassRecordLog：返回的记录
	loc: 记录位置
	minStamp：返回的现有最小的时间戳
	maxStamp：返回的现有最大的时间戳
返回：
	-1/文件错误；0/没有指定的记录；1/成功；
	2/这时，pPassRecordLog是第一个大于指定时间戳的记录
*/
int GetPassRecordLogByStamp(unsigned int timeStamp[], struct SPassRecordLog *pPassRecordLog, 
	unsigned int minStamp[], unsigned int maxStamp[])
{
	int ret = 0;
	pthread_mutex_lock(&g_PassRecordLogInfo.m_Meta.m_lock);
	ret = GetLogByStamp(&g_PassRecordLogInfo, timeStamp, (struct SLog*)pPassRecordLog, minStamp, maxStamp);
	pthread_mutex_unlock(&g_PassRecordLogInfo.m_Meta.m_lock);
	return ret;
}

// 新增： 上线后，将缓存的数据重新发送
//  expire_hour  至多缓存发送n小时的数据, 为0时不发送数据， -1时发送所有的数据
//返回值：为0操作成功，为-1操作失败，请检查
int ResendCachePassRecordLimitByDate(int expire_hour)
{
	if ( expire_hour == 0 )
	{
		return -1;
	}
	struct tm   queryTime;
	time_t      now;
	time_t      querytime;
	struct tm   *temp;
	now = time(NULL);
	now -= expire_hour*3600;
	temp = localtime(&now);
	memcpy(&queryTime, temp, sizeof(struct tm));
	querytime = mktime(&queryTime);

	unsigned int startStamp[2];
	unsigned long long seconds;
	seconds = querytime;
	seconds *= 1000000;
	startStamp[0] = seconds>>32;
	startStamp[1] = seconds&0xffffffff;
	unsigned int minStamp[2]={0,0};
	unsigned int maxStamp[2]={0,0};
	int          ret;

	
	struct SPassRecordLog log;
	ret = GetPassRecordLogByStamp( startStamp, &log, minStamp, maxStamp);
	if ( ret < 0 )
	{
		printf("File Error\n");
		return -1;
	}
	if ( ret == 0 )// no record
	{
		if( minStamp[0] == 0 && minStamp[1] == 0 )
		{
			printf("File Empty!!!\n");
			return -1;
		}
		if ( Compare64( startStamp, minStamp ) <= 0 ) // startStamp < minStamp 
		{
			startStamp[0] = minStamp[0];
			startStamp[1] = minStamp[1];
			ret = GetPassRecordLogByStamp( startStamp, &log, minStamp, maxStamp);
		}
	}
	
	unsigned int  seqNo[2];
	unsigned int  minNo[2];
	unsigned int  maxNo[2];
	int           isFirst=1;// 首次发送的标签

	seqNo[0] = log.m_Meta.m_SeqNo[0];
	seqNo[1] = log.m_Meta.m_SeqNo[1];

	int i = 0;
	unsigned int subval[2] ={0,0};

	//初始化log结构体
	memset( &log,0,sizeof(struct SPassRecordLog) );

	ret = GetPassRecordLogBySeqNo( seqNo, &log, minNo, maxNo);

	//计算起点序号到最大序号差值，即要处理的记录条数
//	Substract64(maxNo,seqNo,subval); 
	
	while(  (Compare64(seqNo, maxNo ) <= 0) && i < 9999 )//subval[1] <= RESEND_RECORD_NUM) // seqNo <= maxNo 
	{
		unsigned int temp[2];
		ret = GetPassRecordLogBySeqNo(seqNo, &log, temp, temp);
		if( log.m_Flag == 1 )
		{
			//将根据序号查找到的记录拷贝到发送缓冲区
			memcpy( &fakedata[i], &log, sizeof(log) );
			i++;
		}
		else
		{
		   	//printf("%llu Already send, skip\n", ((unsigned long long)seqNo[1]<<32) + seqNo[1]);
		}
		Add32(seqNo, 1, temp);
		seqNo[0] = temp[0];
		seqNo[1] = temp[1];
	}

	//批量发送	
	int ret_send;
	ret_send = send_pass_record1(fakedata,i,1);
	if( 0 == ret_send )
	{
		int j;
		unsigned int temp2[2];
		for(j=0;j<i;i++)
		{
			seqNo[0] = fakedata[j].m_Meta.m_SeqNo[0];
			seqNo[1] = fakedata[j].m_Meta.m_SeqNo[1];
			fakedata[j].m_Flag = 0;
			SetPassRecordLogBySeqNo(seqNo, &fakedata[j], temp2, temp2);
		}


	}else{
		return -1;
	}

	return 0;
}
