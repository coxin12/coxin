#include <stdio.h>
#include <unistd.h>

#include "Queue.h"


pthread_mutex_t  g_mutex = PTHREAD_MUTEX_INITIALIZER;

extern "C" typedef void* (*Cvoidp_Func_voidp)(void*);

Queue<int>* GetCleanQueue()
{
    static Queue<int> q2;
    return &q2;
}

void *threadMain( void *parg )
{
    Queue<int> *pTaskQueue = (Queue<int>*)parg;
    Queue<int> *pCleanQueue = GetCleanQueue();

    int *pTask = new int;

    for(;;)
    {
        pTaskQueue->Pop(pTask);
        fprintf(stderr, "thread %ld task %d\n",(long)(pthread_self()),*pTask);
        sleep(3);
        pCleanQueue->Push(*pTask);
    }
}

int create_thread( Queue<int> *&pQueue)
{
    int retval;
    pthread_attr_t attr;

    if ( (retval = pthread_attr_init(&attr)) != 0 )
    {
        return -1;
    }

    if ( (retval = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0 )
    {
        return -1;
    }

    pthread_t tid;
    if ( (retval = pthread_create(&tid, &attr, (Cvoidp_Func_voidp)threadMain, pQueue)) != 0 )
    {
        return -1;
    }
    if ( (retval = pthread_create(&tid, &attr, (Cvoidp_Func_voidp)threadMain, pQueue)) != 0 )
    {
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    int i =1;
    static Queue<int> q1;
    for( ; i< 3; i++)
    {
        q1.Push(i);
    }
    Queue<int> *pTaskQueue = &q1;
    
    create_thread( pTaskQueue );
    int *pTask = new int;

    Queue<int> *pCleanQueue = GetCleanQueue();
    sleep(4);

    while(1)
    {
        int iRes = 0;
        iRes = pCleanQueue->TimePop(pTask);
        sleep(1);
        if( -2 == iRes )
        {
            fprintf(stderr, "(main) pCleanQueue->TimePop(pTask)队列为空等待超时退出....\n");
        }
        if( -1 == iRes )
        {
            fprintf(stderr, "(main) pCleanQueue->TimePop(pTask)出错...\n");
        }
        if( 0 == iRes )
        {
            fprintf(stderr, "(main) pCleanQueue->TimePop(pTask)结束_______pop_end________....\n");
        }

        if( 0 == iRes )
        {
            fprintf(stderr, "(main) pTaskQueue->Push(*pTask)开始_______push_begine______....\n");
            pTaskQueue->Push(*pTask);
            fprintf(stderr, "(main) pTaskQueue->Push(*pTask)结束_______push_end_________....\n");
        }
    }

    return 0;
}

