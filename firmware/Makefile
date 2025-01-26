# Makefile for building the Embedded Audio Player Firmware
# Author: Abay Kulamkadyr

# =============================================================================
# VARIABLES
# =============================================================================

# Target executable name
TARGET = audio_project

# Project paths
PROJECT_ROOT = $(shell pwd)  # Assumes Makefile is at the project root
FIRMWARE_DIR = firmware
SRC_DIR = $(FIRMWARE_DIR)/src
INCLUDE_DIR = $(FIRMWARE_DIR)/include

# Source files
SOURCES = \
    $(SRC_DIR)/main.c \
    $(SRC_DIR)/accelerometer_listener/accelerometer_listener.c \
    $(SRC_DIR)/audio_parsers/wav_parser.c \
    $(SRC_DIR)/drivers/accelerometer.c \
    $(SRC_DIR)/drivers/joystick.c \
    $(SRC_DIR)/drivers/neo_trellis.c \
    $(SRC_DIR)/drivers/potentiometer.c \
    $(SRC_DIR)/drivers/see_saw.c \
    $(SRC_DIR)/drivers/seg_display.c \
    $(SRC_DIR)/joystick_listener/joystick_listener.c \
    $(SRC_DIR)/launch_pad/launch_pad.c \
    $(SRC_DIR)/segment_display_driver/seg_display_driver.c \
    $(SRC_DIR)/shutdown/shutdown.c \
    $(SRC_DIR)/utils/sleep_milliseconds.c \
    $(SRC_DIR)/volume_changer/volume_changer.c \
    $(SRC_DIR)/wave_audio_player/wave_audio_player.c

# Compiler settings
CROSS_TOOL = arm-linux-gnueabihf-
CC_C = $(CROSS_TOOL)gcc
CC_CPP = $(CROSS_TOOL)g++
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -I$(INCLUDE_DIR)
LFLAGS = -L$(HOME)/cmpt433/public/asound_lib_BBB -lpthread -lasound

# Deployment paths
PUBDIR = $(HOME)/cmpt433/public/myApps
DEPLOY_PATH = $(PUBDIR)/Node_modules-copy  # Updated as per new structure

# Node.js server deployment
NODE_PROJECT_NAME = server
NODE_DEPLOY_PATH = $(PUBDIR)/myApps/$(NODE_PROJECT_NAME)-copy

# =============================================================================
# TARGETS
# =============================================================================

# Default target: build firmware, deploy assets and Node.js server
all: firmware deploy node_install

# Build the firmware executable
firmware: $(SOURCES)
	@echo "Building firmware..."
	$(CC_C) $(CFLAGS) $(SOURCES) -o $(PUBDIR)/$(TARGET) $(LFLAGS)
	@echo "Firmware built successfully."

# Copy audio assets to the deployment directory
assets:
	@echo "Deploying audio assets..."
	mkdir -p $(PUBDIR)/assets/music/
	mkdir -p $(PUBDIR)/assets/sound_effects/
	cp assets/music/* $(PUBDIR)/assets/music/
	cp assets/sound_effects/* $(PUBDIR)/assets/sound_effects/
	@echo "Audio assets deployed."

# Deploy Node.js server files
deploy: assets
	@echo "Deploying Node.js server files to $(DEPLOY_PATH)..."
	mkdir -p $(DEPLOY_PATH)
	chmod a+rwx $(DEPLOY_PATH)
	cp -r server/* $(DEPLOY_PATH)
	@echo 'Do not edit any files in this folder; they are copied!' > $(DEPLOY_PATH)/DO_NOT_EDIT-FILES_COPIED.txt
	@echo "Node.js server files deployed."

# Install Node.js dependencies on the deployment path
node_install:
	@echo "Installing required Node.js packages..."
	cd $(DEPLOY_PATH) && npm install
	@echo "Node.js packages installed."

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(PUBDIR)/$(TARGET)
	@echo "Clean complete."

# =============================================================================
# PHONY TARGETS
# =============================================================================

.PHONY: all firmware assets deploy node_install clean

