#include <alsa/asoundlib.h>
#include <alsa/mixer.h> 
#include <pthread.h>
#include <stdbool.h>
#include <alloca.h>
#include "WaveAudioPlayer.h"
#include "Utils/sleepMilliseconds.h"
#include "joystickListener.h"

#define MAX_FILEPATH_SIZE 	        1024
#define DEFAULT_NUM_TRACKS		    30
#define END_OF_TRACK_FLAG           -1
#define SWITCH_NEXT_TRACK_FLAG       1
#define SWITCH_PREVIOUS_TRACK_FLAG   2
#define MAX_INT32_VALUE             2147483647
#define MIN_INT32_VALUE            -2147483648
#define POSITIVE_OVERFLOW           -1
#define NEGATIVE_OVERFLOW           -2

static drwav_int32 integerSum_overflowChecker(drwav_int32* result, drwav_int32 sample1, drwav_int32 sample2)
{
    *result = sample1 + sample2;
    if(sample1 > 0 && sample2 > 0 && *result < 0)
        return POSITIVE_OVERFLOW;
    if(sample1 < 0 && sample2 < 0 && *result > 0)
        return NEGATIVE_OVERFLOW;
    return 0;
}
static wave_samples_data playlist[DEFAULT_NUM_TRACKS]; /*an array of wave audio tracks*/
/******************************************************
 * FUNCTION PROTOTYPES
 ******************************************************/
static void* playbackThread(void *args);
static int playback_selected_track(wave_samples_data *track);
static int direct_loop(snd_pcm_t *handle, wave_samples_data *track);
static int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, unsigned int rate, unsigned int channels);
static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);    
// static void loadTrackMetaData(wave_samples_data *track);
// static void loadTrackIntoMemory(wave_samples_data * track);
static void stop_playback(void);
/******************************************************
 * PARAMETERS TO SET UP AUDIO DEVICE
 ******************************************************/
static snd_pcm_t *handle;
static unsigned long period_size_in_frames; 
static int resample=1;
snd_pcm_sw_params_t *swparams;
snd_pcm_hw_params_t *hwparams;
static snd_pcm_sframes_t buffer_size_in_frames;

static int period_event = 0;                    /* produce poll event after each period */

/******************************************************
 * PARAMETERS FOR THE PLAYLIST 
 ******************************************************/
static int playlist_capacity;                   /*the capacity of the playlist, will dynamically increase if needed*/
static int playlist_size;                       /*number of track currently in the playlist*/
static int playlist_currentPlayingTrackIndex;

/******************************************************
 * PARAMETERS FOR THREADING
 ******************************************************/
static bool resumePlaybackSignal;
static bool terminateSignal;
static bool switch_next_track_signal;
static bool switch_previous_track_signal;
static pthread_t playback_threadID;
static pthread_mutex_t  playlist_mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t 	playlist_empty_condVar=PTHREAD_COND_INITIALIZER;
static pthread_mutex_t  stop_playback_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   resume_playback_cond = PTHREAD_COND_INITIALIZER;
// Module initialization
void WaveAudioPlayer_Init()
{
	int err=0;
    playlist_capacity = DEFAULT_NUM_TRACKS;
	playlist_currentPlayingTrackIndex = 0;
    playlist_size = 0;
    terminateSignal = false;
    switch_next_track_signal = false;
    switch_previous_track_signal = false;
    resumePlaybackSignal = true;
    //Initializing the data structures for tracks
	for(int i=0; i<playlist_capacity; i++)
	{
		playlist[i].meta_data.file_name = NULL;
		playlist[i].meta_data.wave_file_description=NULL;
        playlist[i].right_channel_samples=NULL;
        playlist[i].left_channel_samples=NULL;
        playlist[i].meta_data.num_samples=0;
        playlist[i].right_channel_position=0;
        playlist[i].left_channel_position=0;
	}
    
    //Openning the default pcm audio device for playback
    if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    
    pthread_create(&playback_threadID, NULL, playbackThread,NULL);
}
/******************************************************
 * FUNCTIONS TO SET UP AUDIO DEVICE
 ******************************************************/
static int set_hwparams(snd_pcm_t *handle,
            snd_pcm_hw_params_t *params, unsigned int rate, unsigned int num_channels)
{
    unsigned int rrate,channels;
    channels = num_channels;
    snd_pcm_uframes_t size;
    int err, dir;
    unsigned int buffer_time = 500000;       /* ring buffer length in us */
    unsigned int period_time = 100000; 
    /* choose all parameters */
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
        printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
        return err;
    }
    /* set hardware resampling */
    err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
       if (err < 0) {
        printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the interleaved read/write format */
    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
    if (err < 0) {
        printf("Access type not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the sample format */
    err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE);
       if (err < 0) {
        printf("Sample format not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the count of channels */
    err = snd_pcm_hw_params_set_channels(handle, params, channels);
    if (err < 0){
        printf("Channels count (%u) not available for playbacks: %s\n", channels, snd_strerror(err));
        return err;
    }
    /* set the stream rate */
    rrate = rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
    if (err < 0) {
        printf("Rate %uHz not available for playback: %s\n", rate, snd_strerror(err));
        return err;
    }
    if (rrate != rate) {
        printf("Rate doesn't match (requested %uHz, get %iHz)\n", rate, err);
        return -EINVAL;
    }
    
    /* set the buffer time */
    err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
    if (err < 0) {
        printf("Unable to set buffer time %u for playback: %s\n", buffer_time, snd_strerror(err));
        return err;
    }
    err = snd_pcm_hw_params_get_buffer_size(params, &size);
    if (err < 0) {
        printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
    }
    buffer_size_in_frames = size;
    /* set the period time */
    err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
    if (err < 0) {
        printf("Unable to set period time %u for playback: %s\n", period_time, snd_strerror(err));
        return err;
    }
    err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
    if (err < 0) {
        printf("Unable to get period size for playback: %s\n", snd_strerror(err));
        return err;
    }
    period_size_in_frames = size;
    /* write the parameters to device */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    return 0;
}

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
    int err;
        
    /* get the current swparams */
    err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0) {
        printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size_in_frames / period_size_in_frames) * period_size_in_frames);
       if (err < 0) {
        printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* allow the transfer when at least period_size samples can be processed */
    /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_event ? buffer_size_in_frames : period_size_in_frames);
    if (err < 0) {
        printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
        return err;
    } 
    /* enable period events when requested */
    err = snd_pcm_sw_params_set_period_event(handle, swparams, 1);
    if (period_event) {
        err = snd_pcm_sw_params_set_period_event(handle, swparams, 1);
        if (err < 0) {
            printf("Unable to set period event: %s\n", snd_strerror(err));
            return err;
        }
    } 
    /* write the parameters to the playback device */
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0) {
        printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    
    return 0;
}
 
                /******************************************************
                 * FUNCTIONS TO PLAY AUDIO IN HARDWARE
                 ******************************************************/
// Underrun and suspend recovery
static int xrun_recovery(snd_pcm_t *handle, int err)
{
    if (err == -EPIPE) {    /* under-run */
        err = snd_pcm_prepare(handle);
        if (err < 0)
            printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
        return 0;
    } else if (err == -ESTRPIPE) {
        while ((err = snd_pcm_resume(handle)) == -EAGAIN)
            sleep(1);   /* wait until the suspend flag is released */
        if (err < 0) {
            err = snd_pcm_prepare(handle);
            if (err < 0)
                printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
        }
        return 0;
    }
    return err;
}

static void playback_wav_to_channels(const snd_pcm_channel_area_t *areas, 
              snd_pcm_uframes_t offset,
              int count, wave_samples_data *track)
{

    unsigned int channels= track->meta_data.wave_file_description->channels;
    unsigned char *samples[channels];
    int steps[channels];
    unsigned int chn;
    int format_bits = snd_pcm_format_width(SND_PCM_FORMAT_S32_LE);
    
    int bps = format_bits / 8;  /* bytes per sample */
    int phys_bps = snd_pcm_format_physical_width(SND_PCM_FORMAT_S32_LE) / 8;
    int big_endian = snd_pcm_format_big_endian(SND_PCM_FORMAT_S32_LE) == 1;
    
    /* verify and prepare the contents of areas */
    for (chn = 0; chn < channels; chn++) {
        if ((areas[chn].first % 8) != 0) {
            printf("areas[%u].first == %u, aborting...\n", chn, areas[chn].first);
            exit(EXIT_FAILURE);
        }
        samples[chn] = /*(signed short *)*/(((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
        if ((areas[chn].step % 16) != 0) {
            printf("areas[%u].step == %u, aborting...\n", chn, areas[chn].step);
            exit(EXIT_FAILURE);
        }
        steps[chn] = areas[chn].step / 8;
        samples[chn] += offset * steps[chn];
    }
    /* fill the channel areas */
    while (count-- > 0 && 
          (track->left_channel_position!=track->meta_data.num_samples))
    {
        pthread_mutex_lock(&playlist_mutex);
        for (chn = 0; chn < channels; chn++) {
            /* Generate data in native endian format */
            if (chn==0)
            {
                int32_t res=track->left_channel_samples[track->left_channel_position];
                if (big_endian) {
                    for (int i = 0; i < bps; i++)
                    {
                        *(samples[chn] + phys_bps - 1 - i) = (res>>i*8)& 0xff;
                    }
                    track->left_channel_position+=2;
                } else {
                    for (int i = 0; i < bps; i++)
                    {
                        *(samples[chn] + i) =(res>>i*8) & 0xff;
                    }
                    if(channels>1){
                        track->left_channel_position+=2;
                    } else{
                        track->left_channel_position+=1;
                    }
                }
            }
            if(chn==1){
                int32_t res= track->right_channel_samples[track->right_channel_position];

                if (big_endian) {
                    for (int i = 0; i < bps; i++)
                    {
                        *(samples[chn] + phys_bps - 1 - i) = (res>>i*8)& 0xff;
                    }
                    track->right_channel_position+=2;
                } else {
                    for (int i = 0; i < bps; i++)
                    {    
                        *(samples[chn] + i) =(res>>i*8)& 0xff;
                    }
                    track->right_channel_position+=2;
                }
            }   
            samples[chn] += steps[chn];
        }
        pthread_mutex_unlock(&playlist_mutex);
    }
    
}   
static int direct_loop(snd_pcm_t *handle,
               wave_samples_data *track)
{
    const snd_pcm_channel_area_t *my_areas;
    snd_pcm_uframes_t offset, frames, size;
    snd_pcm_sframes_t avail, commitres;
    snd_pcm_state_t state;
    int err, first = 1;
    while (!switch_next_track_signal && !switch_previous_track_signal && resumePlaybackSignal) {
       
        if(track->left_channel_position==track->meta_data.num_samples){
            track->left_channel_position=0;
            track->right_channel_position=1; 
            return END_OF_TRACK_FLAG;
        }

        state = snd_pcm_state(handle);
        if (state == SND_PCM_STATE_XRUN) {
            err = xrun_recovery(handle, -EPIPE);
            if (err < 0) {
                printf("XRUN recovery failed: %s\n", snd_strerror(err));
                return err;
            }
            first = 1;
        } else if (state == SND_PCM_STATE_SUSPENDED) {
            err = xrun_recovery(handle, -ESTRPIPE);
            if (err < 0) {
                printf("SUSPEND recovery failed: %s\n", snd_strerror(err));
                return err;
            }
        }
        avail = snd_pcm_avail_update(handle);
        if (avail < 0) {
            err = xrun_recovery(handle, avail);
            if (err < 0) {
                printf("avail update failed: %s\n", snd_strerror(err));
                return err;
            }
            first = 1;
            continue;
        }
        if (avail < period_size_in_frames) {
            if (first) {
                first = 0;
                err = snd_pcm_start(handle);
                if (err < 0) {
                    printf("Start error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                }
            } else {
                err = snd_pcm_wait(handle, -1);
                if (err < 0) {
                    if ((err = xrun_recovery(handle, err)) < 0) {
                        printf("snd_pcm_wait error: %s\n", snd_strerror(err));
                        exit(EXIT_FAILURE);
                    }
                    first = 1;
                }
            }
            continue;
        }
        size = period_size_in_frames;
        while (size > 0) {
            frames = size;
            err = snd_pcm_mmap_begin(handle, &my_areas, &offset, &frames);
            if (err < 0) {
                if ((err = xrun_recovery(handle, err)) < 0) {
                    printf("MMAP begin avail error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                }
                first = 1;
            }
            playback_wav_to_channels(my_areas, offset, frames,track);
            commitres = snd_pcm_mmap_commit(handle, offset, frames);
            if (commitres < 0 || (snd_pcm_uframes_t)commitres != frames) {
                if ((err = xrun_recovery(handle, commitres >= 0 ? -EPIPE : commitres)) < 0) {
                    printf("MMAP commit error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                }
                first = 1;
            }
            size -= frames;
        }
    }
    if(switch_next_track_signal){
        switch_next_track_signal=false;
        track->left_channel_position = 0;
        track->right_channel_position = 1;
        return SWITCH_NEXT_TRACK_FLAG;
    }
    if(switch_previous_track_signal){
        switch_previous_track_signal=false;
        track->left_channel_position = 0;
        track->right_channel_position = 1;
        return SWITCH_PREVIOUS_TRACK_FLAG;
    }
    if(!resumePlaybackSignal){
        stop_playback();
    }

    return 0;
}

                /******************************************************
                 * FUNCTIONS TO MANAGE INCOMING REQUESTS TO PLAY AUDIO
                 ******************************************************/


void WaveAudioPlayer_AddTrack( char *wave_file_path)
{
    pthread_mutex_lock(&playlist_mutex);

    //before adding a track need to check if have available space in the datastructure
    //otherwise increase the space

    if(playlist_size > playlist_capacity) {
        printf("playlist is full");
        return;
    }

    char * file_name = (char* )malloc(sizeof(char)* MAX_FILEPATH_SIZE);
    memset(file_name, 0, MAX_FILEPATH_SIZE);
    strncpy(file_name,wave_file_path,MAX_FILEPATH_SIZE);
    //adding the wave file description to the datastructure
    playlist[playlist_size].meta_data.file_name = file_name;
    WaveAudioPlayer_LoadMetaData(&playlist[playlist_size]);
    playlist_size++;
    pthread_mutex_unlock(&playlist_mutex);
    pthread_cond_signal(&playlist_empty_condVar);
}
static void stop_playback(void)
{
    pthread_mutex_lock(&stop_playback_mutex);
    while(!resumePlaybackSignal){
        pthread_cond_wait(&resume_playback_cond, &stop_playback_mutex);
    }
    resumePlaybackSignal = true;
    pthread_mutex_unlock(&stop_playback_mutex);
}
void WaveAudioPlayer_PausePlayback(void)
{
    resumePlaybackSignal = false;
}
void WaveAudioPlayer_ResumePlayback(void)
{
    pthread_mutex_lock(&stop_playback_mutex);
    resumePlaybackSignal = true;
    pthread_mutex_unlock(&stop_playback_mutex);
    pthread_cond_signal(&resume_playback_cond);
}
void WaveAudioPlayer_SkipToNextTrack(void)
{

    switch_next_track_signal = true;

}

void WaveAudioPlayer_ReturnToPreviousTrack(void)
{
    switch_previous_track_signal = true;
    
}
void WaveAudioPlayer_LoadMetaData(wave_samples_data *track)
{
    drwav *wave = (drwav*)malloc(sizeof(drwav));
    //allocating resources for the wave file description
    if(!drwav_init_file(wave, track->meta_data.file_name, NULL)) {
        printf("ERROR: couldn't initialize meta data for the file %s\n", track->meta_data.file_name);
        return;
    }
    //adding the resources to the datastructure
    track->meta_data.wave_file_description = wave;
    track->meta_data.num_samples=wave->totalPCMFrameCount*wave->channels;
}
void WaveAudioPlayer_LoadSamplesIntoMemory(wave_samples_data * track)
{
    drwav *wave = track->meta_data.wave_file_description;
    //reading the samples from the wave file into the left channel
    track->left_channel_samples = malloc(wave->totalPCMFrameCount*wave->channels*sizeof(drwav_int32));
    drwav_read_pcm_frames_s32(wave, wave->totalPCMFrameCount ,track->left_channel_samples);
    //offset for the right channel
    track->right_channel_samples = track->left_channel_samples+1;
    //setting offsets to the channels 
    track->left_channel_position=0;
    track->right_channel_position=1;
}
static int playback_selected_track(wave_samples_data *track)
{
        /*
        if we have reached the end of the track
        move on to the next track in the playlist
        and free the memory allocated for this track
        */
        if (track->left_channel_position>=track->meta_data.num_samples) {
            track->left_channel_position=0;
            track->right_channel_position=1;     
            free(track->left_channel_samples);
            track->left_channel_samples = NULL;
            track->right_channel_samples = NULL;
            return END_OF_TRACK_FLAG;
        }
        /*
         *reload the track metadata and its samples
         *and start playing back the passed track
         */
        if(track->left_channel_samples==NULL || track->right_channel_samples==NULL) {
            printf("Loading wave file into memory %s...\n",track->meta_data.file_name);
            drwav_uninit(track->meta_data.wave_file_description);
            WaveAudioPlayer_LoadMetaData(track);
            WaveAudioPlayer_LoadSamplesIntoMemory(track);            
            printf("Finished loading, playing back %s\n", track->meta_data.file_name);
        }
        pthread_mutex_unlock(&playlist_mutex);
        int flag = direct_loop(handle, track);
        return flag;
}
void set_audio_device_params(unsigned int sampleRate, unsigned int channels)
{
    set_hwparams(handle, hwparams, sampleRate, channels);
    set_swparams(handle, swparams);
}
static void increamentPlaylistPosition()
{
    playlist_currentPlayingTrackIndex++;
    while (playlist_currentPlayingTrackIndex >= playlist_size){playlist_currentPlayingTrackIndex-= playlist_size;}
    while (playlist_currentPlayingTrackIndex < 0){playlist_currentPlayingTrackIndex+=playlist_size;}
}
static void decrementPlaylistPosition()
{
    playlist_currentPlayingTrackIndex--;
    while (playlist_currentPlayingTrackIndex >= playlist_size){playlist_currentPlayingTrackIndex-= playlist_size;}
    while (playlist_currentPlayingTrackIndex < 0){playlist_currentPlayingTrackIndex+=playlist_size;}
}
void* playbackThread(void* arg)
{
    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
	while (!terminateSignal) {
       
        pthread_mutex_lock(&playlist_mutex);
        //wait if the playlist is empty
        while(playlist_size==0) {pthread_cond_wait(&playlist_empty_condVar, &playlist_mutex);}

        //loading the track's metadata
        //set hardware and software parameters based on the wave file
        unsigned int sampleRate = playlist[playlist_currentPlayingTrackIndex].meta_data.wave_file_description->sampleRate;
        unsigned int channels =  playlist[playlist_currentPlayingTrackIndex].meta_data.wave_file_description->channels;
        set_audio_device_params(sampleRate, channels);

        // Generate next block of audio, if we reached the end, update the index to the datastructure
		int returnedFlag = 0;
        returnedFlag = playback_selected_track(&playlist[playlist_currentPlayingTrackIndex]);
        if(returnedFlag == END_OF_TRACK_FLAG || returnedFlag == SWITCH_NEXT_TRACK_FLAG){
            increamentPlaylistPosition();
        } else if(returnedFlag == SWITCH_PREVIOUS_TRACK_FLAG){
            decrementPlaylistPosition();
        }
        printf("Index playing %d\n", playlist_currentPlayingTrackIndex);
       
        //resetting the audio device 
        snd_pcm_close(handle);
        int err = 0;
        if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
            printf("Playback open error: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
        }
        pthread_mutex_unlock(&playlist_mutex);
	}
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    sleep_ms(1);
	return NULL;
}
int WaveAudioPlayer_GetCurrentPosition(void)
{
    int pos=0;
    
    pos = playlist[playlist_currentPlayingTrackIndex].left_channel_position;

    return pos;
}
int WaveAudioPlayer_GetSamplesNum(void)
{   
    int numSamples=0;
    numSamples = playlist[playlist_currentPlayingTrackIndex].meta_data.num_samples;
    return numSamples;
}
void WaveAudioPlayer_RewindForward(void)
{
    int numSample =  playlist[playlist_currentPlayingTrackIndex].meta_data.num_samples;
    int int_increments = numSample *0.05;

    playlist[playlist_currentPlayingTrackIndex].left_channel_position +=int_increments;
    if ( playlist[playlist_currentPlayingTrackIndex].left_channel_position>= numSample)
    {
        playlist[playlist_currentPlayingTrackIndex].left_channel_position = numSample-2;
    }

    playlist[playlist_currentPlayingTrackIndex].right_channel_position = 

    playlist[playlist_currentPlayingTrackIndex].left_channel_position+1;
    if ( playlist[playlist_currentPlayingTrackIndex].right_channel_position>= numSample)
    {
        playlist[playlist_currentPlayingTrackIndex].left_channel_position = numSample-1;
    }
}



void WaveAudioPlayer_RewindBackward(void)
{
    int numSample =  playlist[playlist_currentPlayingTrackIndex].meta_data.num_samples;
    int int_increments = numSample *0.05;

    playlist[playlist_currentPlayingTrackIndex].left_channel_position -=int_increments;
    if ( playlist[playlist_currentPlayingTrackIndex].left_channel_position< 0)
    {
        playlist[playlist_currentPlayingTrackIndex].left_channel_position = 0;
    }

    playlist[playlist_currentPlayingTrackIndex].right_channel_position = 

    playlist[playlist_currentPlayingTrackIndex].left_channel_position+1;
    
   
}

int WaveAudioPlayer_AddPCMSignals(wave_samples_data * track)
{
    pthread_mutex_lock(&playlist_mutex);
    int samplesWritten = 0;
    int startPosition = playlist[playlist_currentPlayingTrackIndex].left_channel_position;
    int endPosition = playlist[playlist_currentPlayingTrackIndex].meta_data.num_samples-1;
    
    for (int i=0; i < track->meta_data.num_samples; i++)
    {
        if((endPosition-startPosition+1) < i) {
            break;
        }
        samplesWritten++;
        drwav_int32 result = 0;
        drwav_int32 ret = integerSum_overflowChecker(&result,playlist[playlist_currentPlayingTrackIndex].left_channel_samples[i+startPosition], track->left_channel_samples[i]);                  
        if(ret == POSITIVE_OVERFLOW) { 
            result = MAX_INT32_VALUE;
        } 
        if(ret == NEGATIVE_OVERFLOW) {
            result = MIN_INT32_VALUE;
        }
        playlist[playlist_currentPlayingTrackIndex].left_channel_samples[i+startPosition] =(drwav_int32) result;
    }
    pthread_mutex_unlock(&playlist_mutex);
    return samplesWritten;
}

void WaveAudioPlayer_Destroy()
{
    printf("Terminating WaveAudioPlayer module...\n");
    switch_next_track_signal = true;
    terminateSignal = true;
    pthread_join(playback_threadID,NULL);
    free(playlist[playlist_currentPlayingTrackIndex].left_channel_samples);
    snd_pcm_drain(handle);
    snd_pcm_close(handle);

    for(int i=0;i<playlist_capacity; i++)
    {
        if(playlist[i].meta_data.file_name!=NULL)
        {
           free(playlist[i].meta_data.file_name);
           drwav_uninit(playlist[i].meta_data.wave_file_description);
        }
    }
    printf("Termination of WaveAudioPlayer module was successfull\n");
    fflush(stdout);
}



