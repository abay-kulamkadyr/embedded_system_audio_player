#ifndef _VOLUMECHANGER_H_
#define _VOLUMECHANGER_H_
#define AUDIOMIXER_MAX_VOLUME 100
/*
	Starts up a thread which changes the volume according to the potentiometer's current value
*/
void VolumeChanger_Init(void);

/*
	Terminates the volume changing thread and releases all the allocated resources
*/
void VolumeChanger_Destroy(void);

#endif