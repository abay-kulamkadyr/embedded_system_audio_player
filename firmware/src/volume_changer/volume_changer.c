#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <alsa/asoundlib.h>
#include <limits.h>
#include <alloca.h> // needed for mixer
#include "../../Include/VolumeChanger/volumeChanger.h"
#include "../../Include/Utils/sleepMilliseconds.h"
#include "../../Include/HardwareControlModule/potentiometer.h"

//Prototypes:
static void* VolumeChanger_ThreadFunc (void * attr);
static void VolumeChanger_setVolume(int newVolume);
static int volume;
static pthread_t volumeChanger_pid;
static bool terminateSignal;

void VolumeChanger_Init()
{
    Potentiometer_Init();
    terminateSignal=false;
    pthread_create(&volumeChanger_pid, NULL, VolumeChanger_ThreadFunc,NULL);
}

static void* VolumeChanger_ThreadFunc (void * attr)
{

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);

	while(!terminateSignal)
	{	
		int POTreading= Potentiometer_getReading();
		int voltageToPercent=(int) ((POTreading/4095.0)*100);
		VolumeChanger_setVolume(voltageToPercent);
	}
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    sleep_ms(1);
    return NULL;
}
/*
	Envokes alsamixer from terminal and sets the volume to volPercent 
*/
static void VolumeChanger_setVolume(int newVolume){
	
	// Ensure volume is reasonable; If so, cache it for later getVolume() calls.
	if (newVolume < 0 || newVolume > AUDIOMIXER_MAX_VOLUME) {
		printf("ERROR: Volume must be between 0 and 100.\n");
		return;
	}
	volume = newVolume;

    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "PCM";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(handle);
}

/*
	Terminates the thread and releases all allocated resources
*/
void VolumeChanger_Destroy()
{
    terminateSignal=true;
    Potentiometer_Destroy();
    pthread_cancel(volumeChanger_pid);
    pthread_join(volumeChanger_pid,NULL);
}