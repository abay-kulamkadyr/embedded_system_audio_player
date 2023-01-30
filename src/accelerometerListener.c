#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "WaveAudioPlayer.h"
#include "Utils/sleepMilliseconds.h"
#include "HardwareControlModule/accelerometer.h"
#define X_DIRECTION_THREASHOLD  0.4
#define Y_DIRECTION_THREASHOLD  0.4
#define Z_DIRECTION_THREASHOLD  1.5
#define ACCELEROMETER_SENSITIVITY 16384.0
static bool terminate_signal;
static pthread_t accelerometer_pid;
static void *accelerationSamplingThread(void *arg);
void AccelerometerListener_Init(void)
{
    Accelerometer_init();
    terminate_signal = false;
    int err= pthread_create(&accelerometer_pid, NULL, accelerationSamplingThread,NULL);
    if(err){
        printf("Failed to create Zen Cape controller thread, terminating the programm...\n");
        exit(-1);
    }
}
static void *accelerationSamplingThread(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
    while(!terminate_signal)
    {
        sleep_ms(10);
        float xDir_value= Accelerometer_getX_Value()/ACCELEROMETER_SENSITIVITY;
        if(xDir_value>X_DIRECTION_THREASHOLD)
        {
            WaveAudioPlayer_SkipToNextTrack();
       
            printf("Y value: %f\t", xDir_value);
         
            //Wait until back to normal acceleration
            
            sleep_ms(100);
        }
        if(xDir_value<-X_DIRECTION_THREASHOLD)
        {
            WaveAudioPlayer_ReturnToPreviousTrack();
            printf("Y value: %f\t", xDir_value);
            sleep_ms(100);
        }
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    sleep_ms(1);
    return NULL;

}

void AccelerometerListener_Destroy(void)
{
    terminate_signal = true;
    pthread_cancel(accelerometer_pid);
    pthread_join(accelerometer_pid,NULL);
    Accelerometer_destroy();
}