/*
 * Supports 8-bit unsigned and 16-bit signed PCM encoded samples
 * using dr_wav library.
 */
#ifndef WAVE_AUDIO_PLAYER_H
#define WAVE_AUDIO_PLAYER_H

#ifndef DR_WAV_IMPLEMENTATION
#include "../../audio_parsers/wav_parser.h"
#define DR_WAV_IMPLEMENTATION
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char*  file_name;
    drwav *wave_file_description;
    int    num_samples;
} wave_meta_data;

typedef struct {
    wave_meta_data meta_data;
    drwav_int32*   right_channel_samples;
    drwav_int32*   left_channel_samples;
    unsigned int   left_channel_position;
    unsigned int   right_channel_position;
} wave_samples_data;

void WaveAudioPlayer_Init(void);
void WaveAudioPlayer_AddTrack(char *wave_file_path);
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
void WaveAudioPlayer_LoadSamplesIntoMemory(wave_samples_data *track);
int  WaveAudioPlayer_AddPCMSignals(wave_samples_data *track);

#endif

