#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "launchPad.h"
#include "WaveAudioPlayer.h"
#include "HardwareControlModule/neoTrellis.h"
#include "Utils/sleepMilliseconds.h"
#include "shutdown.h"
#define RED          (color)        {.red = 255,   .green = 0,     .blue = 0}
#define NEON_YELLOW  (color)        {.red = 244,   .green = 231,   .blue = 34}
#define NEON_RED     (color)        {.red = 210,   .green = 39,    .blue = 48}
#define NEON_FUSCHIA (color)        {.red = 219,   .green = 62,    .blue = 177}
#define NEON_GREEN   (color)        {.red = 68,    .green = 214,   .blue = 44}
#define NEON_BLUE    (color)        {.red = 77,    .green = 77,    .blue = 255}
#define NEON_PURPLE  (color)        {.red = 199,   .green = 36,    .blue = 177}
#define NEON_ORANGE  (color)        {.red = 255,   .green = 173,   .blue = 0}
#define NEON_PINK    (color)        {.red = 254,   .green = 0,     .blue = 246 }
#define NEON_PURPLE_DARK  (color)   {.red = 1,     .green = 30,    .blue = 254}

#define LIGHTEST_GREEN (color)   {.red = 51,     .green = 255,    .blue = 153}
#define BLUE (color)   {.red = 0,     .green = 0,    .blue = 255}
#define ORANGE (color)   {.red = 255,     .green = 178,    .blue = 102}
#define PURPLE (color)   {.red = 255,     .green = 51,    .blue = 255}
#define YELLOW (color)   {.red = 255,     .green = 255,    .blue = 0}
#define PALE (color)   {.red = 255,     .green = 229,    .blue = 204}

 

static bool terminate_signal;
#define BUTTONS_NUM             16
#define MAX_PATH_SIZE           1024
#define AUDIO_FILES_NUM         15
#define MIXING_AUDIO_FILES_PATH "Sound_Effects_Wave_Files/"
#define AUDIO_0                 "128_BouncyDrums_02_726.wav"
#define AUDIO_1                 "128_D#m_DreamySub_01_726.wav"
#define AUDIO_2                 "128_D#m_DreamySynth_01_726.wav"
#define AUDIO_3                 "140_A_DaisyArp_01_726.wav"
#define AUDIO_4                 "140_A_DaisyPad_01_726.wav"
#define AUDIO_5                 "160_B_FutureSynth_01_726.wav"
#define AUDIO_6                 "Cm_Vocal_01_726.wav"
#define AUDIO_7                 "E_Atmosphere_01_726.wav"
#define AUDIO_8                 "100_ D_AcidKindaSynth_849.wav"
#define AUDIO_9                 "100_BouncyDrums_849.wav"
#define AUDIO_10                 "100_CyberpunkDrums_01_849.wav"
#define AUDIO_11                 "100_D_EvilBass_01_849.wav"
#define AUDIO_12                 "100_F_CyberpunkUplifter_849.wav"
#define AUDIO_13                 "140_A_DaisyPad_01_726.wav"
#define AUDIO_14                 "Kick_014_726.wav"
static char audio_filenames [AUDIO_FILES_NUM] [MAX_PATH_SIZE] = {
    {MIXING_AUDIO_FILES_PATH AUDIO_0},
    {MIXING_AUDIO_FILES_PATH AUDIO_1},
    {MIXING_AUDIO_FILES_PATH AUDIO_2},
    {MIXING_AUDIO_FILES_PATH AUDIO_3},
    {MIXING_AUDIO_FILES_PATH AUDIO_4},
    {MIXING_AUDIO_FILES_PATH AUDIO_5},
    {MIXING_AUDIO_FILES_PATH AUDIO_6},
    {MIXING_AUDIO_FILES_PATH AUDIO_7},
    {MIXING_AUDIO_FILES_PATH AUDIO_8},
    {MIXING_AUDIO_FILES_PATH AUDIO_9},
    {MIXING_AUDIO_FILES_PATH AUDIO_10},
    {MIXING_AUDIO_FILES_PATH AUDIO_11},
    {MIXING_AUDIO_FILES_PATH AUDIO_12},
    {MIXING_AUDIO_FILES_PATH AUDIO_13},
    {MIXING_AUDIO_FILES_PATH AUDIO_14},
};

static color buttonColorsMapping[BUTTONS_NUM] = {
            NEON_YELLOW,
            NEON_RED,
            NEON_FUSCHIA,
            NEON_GREEN,
            NEON_BLUE,
            NEON_PURPLE,
            NEON_ORANGE,
            NEON_PINK,
            NEON_PURPLE_DARK,
            LIGHTEST_GREEN,
            BLUE,
            ORANGE,
            PURPLE,
            YELLOW, 
            PALE
};
wave_samples_data wave_datas [AUDIO_FILES_NUM];

static pthread_t keypadListener_pid;
static void *mixingThread(void* arg);

static inline void loadMetaDataAndSamples(wave_samples_data* track)
{
    WaveAudioPlayer_LoadMetaData(track);
    WaveAudioPlayer_LoadSamplesIntoMemory(track);
}
void LaunchPad_Init(void)
{
    printf("Loading data for launch pad\n");
    terminate_signal = false;
	for(int i=0; i < AUDIO_FILES_NUM; i++)
    {
        wave_datas[i].meta_data.file_name = audio_filenames[i];
        loadMetaDataAndSamples(&wave_datas[i]);
    }
    NeoTrellis_LEDs_SetPixel_to_Color(15, RED);
    NeoTrellis_LEDs_UpdateTrellisBuff();
    printf("Finished loading data for launch pad\n");
    pthread_create(&keypadListener_pid, NULL, mixingThread, NULL);
}
static void playWaveAndLightUpPixel(wave_samples_data* wave_data,int index)
{
    WaveAudioPlayer_AddPCMSignals(wave_data);
    NeoTrellis_LEDs_RippleEffect(index, buttonColorsMapping[index]);
    NeoTrellis_LEDs_SetPixel_to_Color(15,RED);
    NeoTrellis_LEDs_UpdateTrellisBuff();
}
static void *mixingThread(void* arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
    while (!terminate_signal)
    {
        int pushedIndex = NeoTrellis_Keys_getPushedButtonIndex();
        if(pushedIndex < AUDIO_FILES_NUM) {
            playWaveAndLightUpPixel(&wave_datas[pushedIndex],pushedIndex);
        }
        if(pushedIndex == 15) { 
            unlock_main_thread();
        }
        sleep_ms(200);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    sleep_ms(1);
    return NULL;
}
void LaunchPad_Destroy(void)
{

    terminate_signal = true;
    pthread_cancel(keypadListener_pid);
    pthread_join(keypadListener_pid, NULL);
    for(int i=0; i < AUDIO_FILES_NUM; i++) {
        free(wave_datas[i].left_channel_samples);
        drwav_uninit(wave_datas[i].meta_data.wave_file_description);
    }

}