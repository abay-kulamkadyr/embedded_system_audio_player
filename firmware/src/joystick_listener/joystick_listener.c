#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#include <unistd.h>
#include "../../Include/JoystickListener/joystickListener.h"
#include "../../Include/HardwareControlModule/joystick.h"
#include "../../Include/WavAudioPlayer/WaveAudioPlayer.h"
#include "../../Include/Utils/sleepMilliseconds.h"
#include "../../Include/SegmentDisplayDriver/segDisplayDriver.h"
#include "../../Include/HardwareControlModule/segDisplay.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#define PORT 12345

pthread_t NetUDP_id;

static bool terminate_signal;
static pthread_t joystick_pid;
static void* JoystickListener_thread(void* arg);
static void* NetworkingUDP(void* args);
bool isPaused;
static int pausefromUDP=0;
static int skipaheadfromUDP=0;
static int skipbackfromUDP=0;
static int forwardtrackfromUDP=0;
static int reversetrackfromUDP=0;

void JoystickListener_init(void)
{   
    JoyStick_init();
	SegDisplayListener_Init();
    terminate_signal = false;
    isPaused = false;
    int err = pthread_create(&joystick_pid, NULL, JoystickListener_thread, NULL);
    if(err) { 
        printf("Failed to create a joystick listening thread\n");
        exit(-1);
    }

}
static void* JoystickListener_thread(void* arg)
{
    pthread_create(&NetUDP_id,NULL,NetworkingUDP,NULL);
    int condition;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
    while(!terminate_signal)
    {
        enum DIRECTION Joystick_currentDirection= JoyStick_getDirection();
        switch (Joystick_currentDirection)
        {
            case DIRECTION_RIGHT:
                printf("right is pushed\n");
                WaveAudioPlayer_SkipToNextTrack();
                sleep_ms(300);
                break;
            case DIRECTION_LEFT:
                printf("left is pushed\n");
                WaveAudioPlayer_ReturnToPreviousTrack();
                sleep_ms(300);
                break;
            
            case DIRECTION_PUSHED:
                if(isPaused){
                    isPaused = false; 
                    WaveAudioPlayer_ResumePlayback();
                    SegDisplayListener_Init();
                }
                else{
                    isPaused= true; 
                    WaveAudioPlayer_PausePlayback();
	                SegDisplayListener_Destroy();
                    DisplayP("ON");
                }
                sleep_ms(300);
                break;
            case DIRECTION_UP:
                condition=0;               
                if(isPaused){
                    condition=1;
                    SegDisplayListener_Init();
                }
                else{
                    WaveAudioPlayer_PausePlayback();
                }
                printf("Direction is up\n");
                while(JoyStick_getDirection()== DIRECTION_UP){
                    sleep_ms(300);
                    WaveAudioPlayer_RewindForward();
                }
             
                if (condition!=1){
                    WaveAudioPlayer_ResumePlayback();
                 
                }
                else{
                	SegDisplayListener_Destroy();
                	DisplayP("ON");
                }
                break;
                
                case DIRECTION_DOWN:
                condition=0;               
                if(isPaused){
                    condition=1;
                    SegDisplayListener_Init();
                }
                else{
                    WaveAudioPlayer_PausePlayback();
                }
                printf("Direction is down\n");
                while(JoyStick_getDirection()== DIRECTION_DOWN){
                    sleep_ms(300);
                    WaveAudioPlayer_RewindBackward();
                }
             
                if (condition!=1){
                    WaveAudioPlayer_ResumePlayback();
                 
                }
                else{
                	SegDisplayListener_Destroy();
                	DisplayP("ON");
                }
                break;
            default:
                break;
        }
        if (forwardtrackfromUDP==1){
        	condition=0;               
                if(isPaused){
                    condition=1;
                    SegDisplayListener_Init();
                }
                else{
                    WaveAudioPlayer_PausePlayback();
                }
		while (forwardtrackfromUDP==1){
			sleep_ms(300);
			WaveAudioPlayer_RewindForward();
		}
		if (condition!=1){
                    WaveAudioPlayer_ResumePlayback();
                 
                }
                else{
                	SegDisplayListener_Destroy();
                	DisplayP("ON");
                }
	}
	else if (reversetrackfromUDP==1){
        	condition=0;               
                if(isPaused){
                    condition=1;
                    SegDisplayListener_Init();
                }
                else{
                    WaveAudioPlayer_PausePlayback();
                }
		while (reversetrackfromUDP==1){
			sleep_ms(300);
			WaveAudioPlayer_RewindBackward();
		}
		if (condition!=1){
                    WaveAudioPlayer_ResumePlayback();
                 
                }
                else{
                	SegDisplayListener_Destroy();
                	DisplayP("ON");
                }
	}
	else if (pausefromUDP==1){
		if(isPaused){
                    isPaused = false; 
                    WaveAudioPlayer_ResumePlayback();
                    SegDisplayListener_Init();
                }
                else{
                    isPaused= true; 
                    WaveAudioPlayer_PausePlayback();
	                SegDisplayListener_Destroy();
                    DisplayP("ON");
                }
                sleep_ms(300);
                pausefromUDP=0;
	}
	else if (skipaheadfromUDP == 1){
		WaveAudioPlayer_SkipToNextTrack();
		skipaheadfromUDP=0;
		sleep_ms(300);
	}
	else if (skipbackfromUDP == 1){
		WaveAudioPlayer_ReturnToPreviousTrack();
		skipbackfromUDP=0;
		sleep_ms(300);
	}
		
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    sleep_ms(1);
    return NULL;
}

void* NetworkingUDP(void* args){
	//struct timespec delay={0,100000000};
	//int* info=(int*)args;
	int sockfd;
	char buffer[256];
	struct sockaddr_in servaddr, cliaddr;
	if ((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0){
		perror("socket creation failed");
		exit(EXIT_FAILURE);
		
	}
	memset(&servaddr,0,sizeof(servaddr));
	memset(&cliaddr,0,sizeof(cliaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=INADDR_ANY;
	servaddr.sin_port=htons(PORT);
	
	if (bind(sockfd,(const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0){
		perror("Bind Failed");
		exit(EXIT_FAILURE);
		
	}
	socklen_t len=sizeof(cliaddr);
	int  n;
	//len=sizeof(cliaddr);
	char str[1500];
	
	while(1){
		n=recvfrom(sockfd,(char*)buffer,1024,MSG_WAITALL,(struct sockaddr*) &cliaddr, &len);
		buffer[n]='\0';
		//pause and unpause
		if (strcmp(buffer,"P\n")==0){
			pausefromUDP=1;
			sprintf(str,"Done\n");
			
		}
		//skip track ahead
		else if (strcmp(buffer,"SH\n")==0){
			skipaheadfromUDP=1;
			sprintf(str,"Done\n");
			
		}
		//skip track back
		else if (strcmp(buffer,"SB\n")==0){
			skipbackfromUDP=1;
			sprintf(str,"Done\n");
			
		}
		//reverse in track
		else if (strcmp(buffer,"RT\n")==0){
			if (reversetrackfromUDP==0){
				reversetrackfromUDP=1;
			}
			else{
				reversetrackfromUDP=0;
			}
			sprintf(str,"Done\n");
			
		}
		//reverse in track
		else if (strcmp(buffer,"FT\n")==0){
			if (forwardtrackfromUDP==0){
				forwardtrackfromUDP=1;
			}
			else{
				forwardtrackfromUDP=0;
			}
			sprintf(str,"Done\n");
			
		}

		// else if (strcmp(buffer,"VU\n")==0){
		// 	//calling the vol increase
		// 	sprintf(str,"Done\n");
			
		// }
		
        // else if (strcmp(buffer,"VD\n")==0){
		// 	//calling the vol decrease
		// 	sprintf(str,"Done\n");
			
		// }
		// else if(strcmp(buffer,"displayVolume\n")==0){
        //     char messageTx[4];
        //     FILE* upTime_fptr;
        //     char buff[28];
    
        //     sprintf(messageTx, ":%d", getVolume());
            
        //     sendto( socketDescriptor,
        //         messageTx, strlen(messageTx),
        //         0,
        //         (struct sockaddr *) &sinRemote, sin_len);
        // }
		sendto(sockfd,(const char*)str,strlen(str),MSG_CONFIRM,(const struct sockaddr*) &cliaddr,len);
		
		
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
