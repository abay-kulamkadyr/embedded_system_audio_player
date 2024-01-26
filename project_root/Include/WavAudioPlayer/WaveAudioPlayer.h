/*
Supports only 8-bit unsigned and 16-bit signed PCM encoded samples 
*/
#ifndef _WAVE_AUDIOPLAYER_H_
#define _WAVE_AUDIOPLAYER_H_
#ifndef DR_WAV_IMPLEMENTATION
#include "../AudioParcers/wav_parcer.h"
#define DR_WAV_IMPLEMENTATION
#endif
/******************************************************
 * DATA STRUCTURES FOR WAVE AUDIO
 ******************************************************/
typedef struct{
    char* file_name;
    drwav *wave_file_description;   /*the metadata extracted from the header of the wave file*/
    int num_samples;                /*total number of samples*/
}wave_meta_data;

typedef struct{
	wave_meta_data meta_data;    
    drwav_int32* right_channel_samples;             /*pointer to right channel samples*/
    drwav_int32* left_channel_samples;              /*pointer to left channel samples*/
    unsigned int left_channel_position;         /*offset to the left channel, if one channel audio will just use this pointer*/
    unsigned int right_channel_position;        /*offset to the right channel*/
}wave_samples_data;

void WaveAudioPlayer_Init();
void WaveAudioPlayer_AddTrack(char *wave_file_path);

//void WaveAudioPlaer_PlayTrackNumber(int trackNum);
void WaveAudioPlayer_PausePlayback(void);
void WaveAudioPlayer_ResumePlayback(void);
void WaveAudioPlayer_SkipToNextTrack(void);
void WaveAudioPlayer_ReturnToPreviousTrack(void);
int  WaveAudioPlayer_GetCurrentPosition(void);
int  WaveAudioPlayer_GetSamplesNum(void);
void WaveAudioPlayer_RewindForward(void);
void WaveAudioPlayer_RewindBackward(void);
void WaveAudioPlayer_Destroy(void);
void WaveAudioPlayer_LoadMetaData(wave_samples_data *track);
void WaveAudioPlayer_LoadSamplesIntoMemory(wave_samples_data * track);
int  WaveAudioPlayer_AddPCMSignals(wave_samples_data * track);

#endif
