#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../include/accelerometer_listener/accelerometer_listener.h"
#include "../../include/wave_audio_player/wave_audio_player.h"
#include "../../include/utils/sleep_milliseconds.h"
#include "../../include/drivers/accelerometer.h"

#define X_DIRECTION_THRESHOLD  0.4f
#define ACCELEROMETER_SENSITIVITY 16384.0f

static bool terminate_signal;
static pthread_t accelerometer_pid;
static void *accelerationSamplingThread(void *arg);

void AccelerometerListener_Init(void)
{
    Accelerometer_init();
    terminate_signal = false;
    int err = pthread_create(&accelerometer_pid, NULL, accelerationSamplingThread, NULL);
    if (err) {
        printf("Failed to create accelerometer listener thread, terminating...\n");
        exit(-1);
    }
}

static void *accelerationSamplingThread(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    while (!terminate_signal) {
        sleep_ms(10);
        float xDir_value = Accelerometer_getX_Value() / ACCELEROMETER_SENSITIVITY;

        // Tilt forward => next track
        if (xDir_value > X_DIRECTION_THRESHOLD) {
            WaveAudioPlayer_SkipToNextTrack();
            printf("Accelerometer x: %f -> Next Track\n", xDir_value);
            sleep_ms(100);
        }
        // Tilt backward => previous track
        else if (xDir_value < -X_DIRECTION_THRESHOLD) {
            WaveAudioPlayer_ReturnToPreviousTrack();
            printf("Accelerometer x: %f -> Previous Track\n", xDir_value);
            sleep_ms(100);
        }
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    sleep_ms(1);
    return NULL;
}

void AccelerometerListener_Destroy(void)
{
    terminate_signal = true;
    pthread_cancel(accelerometer_pid);
    pthread_join(accelerometer_pid, NULL);
    Accelerometer_destroy();
}

