
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include "../../include/segment_display_driver/segment_display_driver.h"
#include "../../include/drivers/segment_display.h"
#include "../../include/utils/sleep_milliseconds.h"
#include "../../include/wave_audio_player/wave_audio_player.h"

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
