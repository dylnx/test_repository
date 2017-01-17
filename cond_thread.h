#ifndef COND_THREAD_H_7KPRUOCM
#define COND_THREAD_H_7KPRUOCM

#include <pthread.h>

typedef void (*thread_callback)(void *args);

struct _cond_thread_msg
{
	struct _cond_thread_msg *m_next;
	int                     status;
	void                    *data;
};

struct CondThread
{
	struct _cond_thread_msg *workq;
	struct _cond_thread_msg *qtail; // 队列尾指针
	int                      flag;
	int                      qlen; // 队列长度
	pthread_t                pid;
	pthread_mutex_t          qlock;
	pthread_cond_t           qready;
	thread_callback          consumer;
};

struct CondThread *CreateCondThread(thread_callback func);
void ThreadRun(struct CondThread *thread);
void TriggerThread(struct CondThread *thread, int status, void *data);

#endif /* end of include guard: COND_THREAD_H_7KPRUOCM */
