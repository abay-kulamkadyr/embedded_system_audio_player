# Makefile for building embedded application.
# by Brian Fraser

# Edit this file to compile extra C files into their own programs.
TARGET= audio_project


PROJECT_NAME=Project-server
DEPLOY_PATH= $(HOME)/cmpt433/public/myApps/$(PROJECT_NAME)-copy

SOURCES= main.c ./HardwareControlModule/accelerometer.c ./HardwareControlModule/potentiometer.c WaveAudioPlayer.c shutdown.c ./Utils/sleepMilliseconds.c dr_wav.h volumeChanger.c ./HardwareControlModule/joystick.c joystickListener.c accelerometerListener.c segDisplayDriver.c ./HardwareControlModule/segDisplay.c ./HardwareControlModule/neoTrellis.c ./HardwareControlModule/seeSaw.c launchPad.c


PUBDIR = $(HOME)/cmpt433/public/myApps
OUTDIR = $(PUBDIR)
CROSS_TOOL = arm-linux-gnueabihf-
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror 


# Asound process:
# get alibsound2 lib on target:
# 	# apt-get install libasound2
# Copy target's /usr/lib/arm-linux-gnueabihf/libasound.so.2.0.0 
#      to host  ~/public/asound_lib_BBB/libasound.so
# Copy to just base library:

LFLAGS = -L$(HOME)/cmpt433/public/asound_lib_BBB


# -pg for supporting gprof profiling.
#CFLAGS += -pg



all: wav deploy  node_install
	$(CC_C) $(CFLAGS) $(SOURCES) -o $(OUTDIR)/$(TARGET)  $(LFLAGS) -lpthread -lasound

# Copy wave files to the shared folder
test: 
	$(CC_C) $(CFLAGS) controlsExample.c -o $(OUTDIR)/test  $(LFLAGS) -lpthread -lasound -lm
wav:
	mkdir -p $(PUBDIR)/Playback_Wave_Files/
	mkdir -p $(PUBDIR)/Sound_Effects_Wave_Files/
	cp Playback_Wave_Files/* $(PUBDIR)/Playback_Wave_Files/ 
	cp Sound_Effects_Wave_Files/* $(PUBDIR)/Sound_Effects_Wave_Files/ 
clean:
	rm -f $(OUTDIR)/$(TARGET)

deploy:
	@echo 'COPYING THE NODE.JS FILES TO $(DEPLOY_PATH)'
	@echo ''
	mkdir -p $(DEPLOY_PATH)
	chmod a+rwx $(DEPLOY_PATH)
	cp -r Project-server/* $(DEPLOY_PATH)
	@echo 'Do not edit any files in this folder; they are copied!' > $(DEPLOY_PATH)/DO_NOT_EDIT-FILES_COPIED.txt
	@echo ''
	@echo 'NOTE: On the host, in $(DEPLOY_PATH), it is best to run: npm install'
	@echo '      Or, just run the node_install target in this makefile.'

node_install:
	@echo ''
	@echo ''
	@echo 'INSTALLING REQUIRED NODE PACKAGES'
	@echo '(This may take some time)'
	@echo ''
	cd $(DEPLOY_PATH) && npm install
