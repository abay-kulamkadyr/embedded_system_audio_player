
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include "../../Include/SegmentDisplayDriver/segDisplayDriver.h"
#include "../../Include/HardwareControlModule/segDisplay.h"
#include "../../Include/Utils/sleepMilliseconds.h"
#include "../../Include/WavAudioPlayer/WaveAudioPlayer.h"

static bool terminate;
pthread_t segDisplay_pid;
void * segDisplayListener_thread(void* arg);
void SegDisplayListener_Init(void)
{
    terminate = false;
    InitializeSegDisplay();
    int err = pthread_create(&segDisplay_pid, NULL, segDisplayListener_thread,NULL);
    if(err)
    {
        printf("error creating a seg display thread\n");
    }
}
void * segDisplayListener_thread(void* arg)
{   
    int currpos=0;
    int numSamples=0;

    while(!terminate)
    {   
        sleep_ms(2);
        numSamples=WaveAudioPlayer_GetSamplesNum();
        currpos =  WaveAudioPlayer_GetCurrentPosition();
        DisplayProgressOnSeg(currpos,numSamples);

    }
	return NULL;
}

void SegDisplayListener_Destroy(void)
{
    terminate = true;
    pthread_join(segDisplay_pid, NULL);
}