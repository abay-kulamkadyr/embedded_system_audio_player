#include <stdio.h>
#include <stdlib.h>
#include "shutdown.h"

static pthread_mutex_t* main_thread_mutex; 
static pthread_cond_t* main_thread_condvar;
void lock_main_thread(pthread_mutex_t* mutex, pthread_cond_t *cond_var)
{
    main_thread_mutex= mutex;
    main_thread_condvar= cond_var;
    if(mutex!=NULL && cond_var!=NULL)
    {
        pthread_mutex_lock(main_thread_mutex);
        pthread_cond_wait(cond_var, mutex);
        pthread_mutex_unlock(main_thread_mutex);
    }
    else
    {
        printf("Error, mutex or condvar is pointing to NULL\n");
        exit(-1);
    }
}
void unlock_main_thread()
{
    if (main_thread_mutex!=NULL)
    {
        pthread_mutex_lock(main_thread_mutex);
        pthread_cond_signal(main_thread_condvar);
        pthread_mutex_unlock(main_thread_mutex);
       
    }
    else
    {
        printf("ERROR, couldn't release the lock for main thread\n");
        exit(-1);
    }
}