/*
 *  Small program to read a 16-bit, signed, 44.1kHz wave file and play it.
 *  Written by Brian Fraser, heavily based on code found at:
 *  http://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_min_8c-example.html
 */


#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/launch_pad/launch_pad.h"
#include "../include/wave_audio_player/wave_audio_player.h"
#include "../include/utils/sleep_milliseconds.h"
#include "../include/joystick_listener/joystick_listener.h"
#include "../include/accelerometer_listener/accelerometer_listener.h"
#include "../include/segment_display_driver/segment_display_driver.h"
#include "../include/audio_parsers/wav_parcer.h"
#include "../include/drivers/neo_trellis.h"
#include "../include/shutdown/shutdown.h"
#include "../include/volume_changer/volume_changer.h"

#define SOURCE_FILE1 "../../assets/music/209456_bigjoedrummer_hip-hop-kit-beats (online-audio-converter.com).wav"              
#define SOURCE_FILE2 "../../assets/music/323250__scale75__hip-hop-beats-techno.wav"
#define SOURCE_FILE3 "../../assets/music/510946_theoter_hip-hop-beat (online-audio-converter.com).wav"
#define SOURCE_FILE4 "../../assets/music/bensound-anewbeginning.wav"
#define SOURCE_FILE5 "../../assets/music/bensound-creativeminds.wav"
#define SOURCE_FILE6 "../../assets/music/brilliant-life-30sec.wav"
#define SOURCE_FILE7 "../../assets/music/CantinaBand60.wav"





int main(void)
{
	char input1[1024];
	sprintf(input1,SOURCE_FILE1);
	
	char input2[1024];
	sprintf(input2, SOURCE_FILE2);

	char input3[1024];
	sprintf(input3, SOURCE_FILE3);

	char input4[1024];
	sprintf(input4, SOURCE_FILE4);
	
	char input5[1024];
	sprintf(input5, SOURCE_FILE5);
	
	char input6[1024];
	sprintf(input6, SOURCE_FILE6);
	
	char input7[1024];
	sprintf(input7, SOURCE_FILE7);

	/*
	 *Initilizing modules
	 ********************************/
	NeoTrellis_LEDs_Init();
	WaveAudioPlayer_Init();
	VolumeChanger_Init();
	AccelerometerListener_Init();
	JoystickListener_init();
	LaunchPad_Init();
	//*******************************
	
	
	WaveAudioPlayer_AddTrack(input1);
	WaveAudioPlayer_AddTrack(input2);
	WaveAudioPlayer_AddTrack(input3);
	WaveAudioPlayer_AddTrack(input4);
	WaveAudioPlayer_AddTrack(input5);
	WaveAudioPlayer_AddTrack(input6);
	WaveAudioPlayer_AddTrack(input7);



	/*
	 *Blocking main thread until someone wishes to terminate the program
	 ********************************************************************/
	pthread_mutex_t main_block_mutex=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t main_block_condvar=PTHREAD_COND_INITIALIZER;
	lock_main_thread(&main_block_mutex, &main_block_condvar);
	//*******************************************************************
	LaunchPad_Destroy();
	VolumeChanger_Destroy();
	JoystickListener_destroy();
	AccelerometerListener_Destroy();
	WaveAudioPlayer_Destroy();
	return 0;
}

