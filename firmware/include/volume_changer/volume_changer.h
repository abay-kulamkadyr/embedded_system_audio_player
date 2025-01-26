
#ifndef VOLUME_CHANGER_H
#define VOLUME_CHANGER_H

#define AUDIOMIXER_MAX_VOLUME 100

/*
 * Starts a thread which changes the volume according to the potentiometer's value.
 */
void VolumeChanger_Init(void);

/*
 * Terminates the volume-changing thread and releases all resources.
 */
void VolumeChanger_Destroy(void);

#endif

