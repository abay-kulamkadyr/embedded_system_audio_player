
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "joystick_listener.h"
#include "../../include/drivers/joystick.h"
#include "../../include/wave_audio_player/wave_audio_player.h"
#include "../../include/utils/sleep_milliseconds.h"
#include "../../include/segment_display_driver/seg_display_driver.h"
#include "../../include/drivers/seg_display.h"

#define PORT 12345

static bool terminate_signal;
static pthread_t joystick_pid;
static pthread_t NetUDP_id;
static bool isPaused;
static int pausefromUDP           = 0;
static int skipaheadfromUDP       = 0;
static int skipbackfromUDP        = 0;
static int forwardtrackfromUDP    = 0;
static int reversetrackfromUDP    = 0;

static void* JoystickListener_thread(void* arg);
static void* NetworkingUDP(void* args);

void JoystickListener_init(void)
{
    JoyStick_init();
    SegDisplayListener_Init();
    terminate_signal = false;
    isPaused = false;

    int err = pthread_create(&joystick_pid, NULL, JoystickListener_thread, NULL);
    if (err) {
        printf("Failed to create joystick listening thread\n");
        exit(-1);
    }
}

static void* JoystickListener_thread(void* arg)
{
    pthread_create(&NetUDP_id, NULL, NetworkingUDP, NULL);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    while (!terminate_signal) {
        enum DIRECTION dir = JoyStick_getDirection();
        switch (dir) {
            case DIRECTION_RIGHT:
                printf("Joystick right -> Next track\n");
                WaveAudioPlayer_SkipToNextTrack();
                sleep_ms(300);
                break;
            case DIRECTION_LEFT:
                printf("Joystick left -> Previous track\n");
                WaveAudioPlayer_ReturnToPreviousTrack();
                sleep_ms(300);
                break;
            case DIRECTION_PUSHED:
                if (isPaused) {
                    isPaused = false;
                    WaveAudioPlayer_ResumePlayback();
                    SegDisplayListener_Init();
                } else {
                    isPaused = true;
                    WaveAudioPlayer_PausePlayback();
                    SegDisplayListener_Destroy();
                    DisplayP("ON");
                }
                sleep_ms(300);
                break;
            case DIRECTION_UP:
            {
                int condition = 0;
                if (isPaused) {
                    condition = 1;
                    SegDisplayListener_Init();
                } else {
                    WaveAudioPlayer_PausePlayback();
                }
                printf("Joystick up -> Rewind forward\n");
                while (JoyStick_getDirection() == DIRECTION_UP) {
                    sleep_ms(300);
                    WaveAudioPlayer_RewindForward();
                }
                if (!condition) {
                    WaveAudioPlayer_ResumePlayback();
                } else {
                    SegDisplayListener_Destroy();
                    DisplayP("ON");
                }
            }
                break;
            case DIRECTION_DOWN:
            {
                int condition = 0;
                if (isPaused) {
                    condition = 1;
                    SegDisplayListener_Init();
                } else {
                    WaveAudioPlayer_PausePlayback();
                }
                printf("Joystick down -> Rewind backward\n");
                while (JoyStick_getDirection() == DIRECTION_DOWN) {
                    sleep_ms(300);
                    WaveAudioPlayer_RewindBackward();
                }
                if (!condition) {
                    WaveAudioPlayer_ResumePlayback();
                } else {
                    SegDisplayListener_Destroy();
                    DisplayP("ON");
                }
            }
                break;
            default:
                break;
        }

        // Process UDP triggers
        if (forwardtrackfromUDP == 1) {
            int condition = 0;
            if (isPaused) {
                condition = 1;
                SegDisplayListener_Init();
            } else {
                WaveAudioPlayer_PausePlayback();
            }
            while (forwardtrackfromUDP == 1) {
                sleep_ms(300);
                WaveAudioPlayer_RewindForward();
            }
            if (!condition) {
                WaveAudioPlayer_ResumePlayback();
            } else {
                SegDisplayListener_Destroy();
                DisplayP("ON");
            }
        } else if (reversetrackfromUDP == 1) {
            int condition = 0;
            if (isPaused) {
                condition = 1;
                SegDisplayListener_Init();
            } else {
                WaveAudioPlayer_PausePlayback();
            }
            while (reversetrackfromUDP == 1) {
                sleep_ms(300);
                WaveAudioPlayer_RewindBackward();
            }
            if (!condition) {
                WaveAudioPlayer_ResumePlayback();
            } else {
                SegDisplayListener_Destroy();
                DisplayP("ON");
            }
        } else if (pausefromUDP == 1) {
            if (isPaused) {
                isPaused = false;
                WaveAudioPlayer_ResumePlayback();
                SegDisplayListener_Init();
            } else {
                isPaused = true;
                WaveAudioPlayer_PausePlayback();
                SegDisplayListener_Destroy();
                DisplayP("ON");
            }
            sleep_ms(300);
            pausefromUDP = 0;
        } else if (skipaheadfromUDP == 1) {
            WaveAudioPlayer_SkipToNextTrack();
            skipaheadfromUDP = 0;
            sleep_ms(300);
        } else if (skipbackfromUDP == 1) {
            WaveAudioPlayer_ReturnToPreviousTrack();
            skipbackfromUDP = 0;
            sleep_ms(300);
        }
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    sleep_ms(1);
    return NULL;
}

static void* NetworkingUDP(void* args)
{
    int sockfd;
    char buffer[256];
    struct sockaddr_in servaddr, cliaddr;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port        = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    socklen_t len = sizeof(cliaddr);
    while (1) {
        int n = recvfrom(sockfd, (char*)buffer, 1024, MSG_WAITALL,
                         (struct sockaddr*)&cliaddr, &len);
        buffer[n] = '\0';

        char str[1500];
        if (strcmp(buffer, "P\n") == 0) {
            pausefromUDP = 1;
            sprintf(str, "Done\n");
        } else if (strcmp(buffer, "SH\n") == 0) {
            skipaheadfromUDP = 1;
            sprintf(str, "Done\n");
        } else if (strcmp(buffer, "SB\n") == 0) {
            skipbackfromUDP = 1;
            sprintf(str, "Done\n");
        } else if (strcmp(buffer, "RT\n") == 0) {
            reversetrackfromUDP = !reversetrackfromUDP;
            sprintf(str, "Done\n");
        } else if (strcmp(buffer, "FT\n") == 0) {
            forwardtrackfromUDP = !forwardtrackfromUDP;
            sprintf(str, "Done\n");
        } else {
            sprintf(str, "Unknown\n");
        }
        sendto(sockfd, (const char*)str, strlen(str),
               MSG_CONFIRM, (const struct sockaddr*)&cliaddr, len);
    }
    close(sockfd);
    return NULL;
}

void JoystickListener_destroy(void)
{
    terminate_signal = true;
    pthread_cancel(joystick_pid);
    pthread_join(joystick_pid, NULL);
    JoyStick_destroy();
}

