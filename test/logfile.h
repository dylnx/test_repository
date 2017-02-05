#ifndef LOGFILE_H_AR69HGX7
#define LOGFILE_H_AR69HGX7
#include <pthread.h>
#define MAX_TIMESTAMP_LEN 20

/*
struct SLog
{
	unsigned int        m_SeqNo[2];
	unsigned int        m_TimeStamp[2];
};

struct SPassRecordLog
{
	struct SLog    m_Meta;
	char           m_Tid[20];
	char           m_TimeStamp[20];
	int            m_Channel; // 通行点位
	int            m_Direction;// 通行方向
	int            m_PassResult;//通行结果
	int            m_Flag;
};
*/


struct LogFileMeta
{
	unsigned int    m_RecordSize; //记录的大小
	unsigned int    m_MaxRecord;// 最大记录数
	unsigned int    m_MinRecord;// 最小记录数
	unsigned int    m_BufRecord; // 缓存记录数
	char            m_FileName[500]; //文件名
	FILE      *     m_Fp; // 文件指针
	pthread_mutex_t m_lock; //多线程访问加锁
};

struct LogFile
{
	struct LogFileMeta m_Meta;
	unsigned int    m_TotalCnt; // 最大记录数目
	unsigned int	m_CurrentCnt; //当前记录数目
	unsigned int	m_First; //第一条记录位置
	unsigned int	m_Last; //最后一条记录位置
	unsigned int	m_minSeq[2]; //第一条记录序号
	unsigned int	m_maxSeq[2]; //最后一条记录序号
	unsigned int	m_minStamp[2]; //第一条记录时间戳
	unsigned int	m_maxStamp[2]; //最后一条记录时间戳
};

// 通行记录日志文件
//
// 文件中记录的最大数目
#define MAX_RECORD_LOG_FILE_REC     (1024*512)
// 文件中记录的最小数目
#define MIN_RECORD_LOG_FILE_REC     (1024*10)
// 内存中记录的最大数目
#define MAX_RECORD_LOG_BUF          256
// 日志文件名
#define RECORD_FILE_NAME "PassRecordLog.bin"
extern struct LogFile g_PassRecordLogInfo;


// 通信记录日志操作
int InitPassRecordLogFile();
int ClearPassRecordLog();
void InsertPassRecordLog1(struct SPassRecordLog *pPassRecordLog);
void InsertPassRecordLog2(struct SPassRecordLog *pPassRecordLog);
int WriteBackPassRecordLog(struct SPassRecordLog *pPassRecordLog);
void PassRecordLogHandle(void *args);
void PassRecordSendHandle(void *args);
int InsertPassRecordLogFile(struct SPassRecordLog *pPassRecordLog);
int GetPassRecordLogBySeqNo(unsigned int seqNo[], struct SPassRecordLog *pPassRecordLog,
		unsigned int minNo[], unsigned int maxno[]);
int GetPassRecordLogByStamp(unsigned int timeStamp[], struct SPassRecordLog *pPassRecordLog,
		unsigned int minStamp[], unsigned int maxStamp[]);


int ResendCachePassRecordLimitByDate(int expire_hour);

#endif /* end of include guard: LOGFILE_H_AR69HGX7 */
