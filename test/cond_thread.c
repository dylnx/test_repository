#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cond_thread.h"


struct CondThread *CreateCondThread(thread_callback func)
{
	struct CondThread *thread = 
		(struct CondThread*)malloc(sizeof(struct CondThread));
	memset(thread, 0x00, sizeof(struct CondThread));
	pthread_mutex_init(&thread->qlock, NULL);
	pthread_cond_init(&thread->qready, NULL);
	thread->consumer = func;
	return thread;
}

void _ThreadFunc(void *args)
{
	if ( args == NULL ) return ;
	struct CondThread *thread = args;
	struct _cond_thread_msg *mp;
	thread->flag = 1;

	while(thread->flag){
		pthread_mutex_lock(&thread->qlock);
		while(thread->workq == NULL)
		{
			pthread_cond_wait(&thread->qready, &thread->qlock);
		}
		mp = thread->workq;
		thread->workq = mp->m_next;
		if ( thread->workq == NULL )// 当最后一个元素被移除时， 更新尾指针
		{
			thread->qtail = NULL;
		}
		// Out Queue
		thread->qlen--;
		pthread_mutex_unlock(&thread->qlock);
//		printf("Out Queue %d %d\n", mp->status, (int)thread->qlen);
		if( thread->consumer )
			thread->consumer(mp);
		free(mp);
	}
}


void ThreadRun(struct CondThread *thread)
{
	if ( thread == NULL ) return ;
	pthread_create(&thread->pid, NULL, (void*)_ThreadFunc, (void*)thread);
}

void TriggerThread(struct CondThread *thread, int status, void *data)
{
	if ( thread == NULL ) return ;
	if ( thread->qlen > 1024 ) // 队列长度超过1024则丢弃当前数据
	{
		printf("Queue Full Size %d\n", thread->qlen);
		return;
	}
	pthread_mutex_lock(&thread->qlock);
	struct _cond_thread_msg *msg = 
		(struct _cond_thread_msg*)malloc(sizeof(struct _cond_thread_msg));
	memset(msg, 0x00 , sizeof(struct _cond_thread_msg));
	msg->status = status;
	msg->data = data;
	msg->m_next = NULL;
	thread->qlen++;
	if ( thread->workq == NULL )//队列为空
	{
		thread->workq = msg;
		thread->qtail = msg;
	}
	else
	{
		thread->qtail->m_next = msg;//加到队列尾部
		thread->qtail = msg; //更新尾指针
	}
	//thread->workq = msg;
//	printf("Input %d %d\n", msg->status, (int)msg->m_next);
	pthread_mutex_unlock(&thread->qlock);
	pthread_cond_signal(&thread->qready);
}
