/*
 * Module to block and unblock the main thread.
 * A module can signal the main thread that it's time to finish the program
 * by calling unlock_main_thread().
 */
#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include <pthread.h>

// Main thread blocks on its mutex passed to the function
void lock_main_thread(pthread_mutex_t* mutex, pthread_cond_t* cond_var);
// Unlock the main thread's mutex
void unlock_main_thread(void);

#endif

