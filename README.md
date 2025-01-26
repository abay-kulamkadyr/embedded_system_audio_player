 # 🎵 Embedded Audio Player with Node.js Server 🎵

This repository showcases an **embedded audio player** running on the BeagleBone (Green/Black) platform. It supports real-time .wav file playback, volume control, track navigation, and on-the-fly sound mixing triggered by various inputs (joystick, accelerometer, NeoTrellis button matrix). Additionally, a **Node.js web server** provides a remote interface for track management and control.


---

## 🚀 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Hardware Requirements](#hardware-requirements)
- [Installation](#installation)
  - [Firmware Setup](#firmware-setup)
  - [Node.js Server Setup](#nodejs-server-setup)
- [Usage](#usage)
- [Usage Flow](#usage-flow)
- [Known Limitations & Future Improvements](#known-limitations--future-improvements)

---

## 🌟 Overview

The **Embedded Audio Player** is a versatile application designed to run on the BeagleBone platform. It facilitates real-time playback of `.wav` files, dynamic volume adjustments, track navigation, and on-the-fly sound mixing. User interactions are captured through various input devices, including a joystick, accelerometer, and a NeoTrellis button matrix. The integrated **Node.js web server** offers a remote interface for managing tracks and controlling playback.

---

## ✨ Features

- **Real-Time WAV Playback**
  - Stream `.wav` files using ALSA (`libasound`).
  - Supports both stereo and mono audio formats.

- **Dynamic Sound Mixing**
  - Trigger sound effects via NeoTrellis button presses, overlaying them onto the current track.

- **Hardware Drivers**
  - **Accelerometer:** Detects tilts to skip tracks.
  - **Joystick:** Enables forward/backward navigation through playlists.
  - **NeoTrellis:** Manages a 4×4 RGB LED matrix for button inputs and visual feedback.
  - **Potentiometer:** Allows real-time volume adjustments.
  - **SeeSaw Chip:** Facilitates low-level I2C communications.
  - **14-Segment Display:** Shows track progress and status updates.

- **Multi-Threaded Listeners**
  - **Accelerometer Listener:** Continuously monitors accelerometer data in a separate thread.
  - **Joystick Listener:** Handles joystick inputs for playback control.
  - **Launch Pad:** Manages NeoTrellis keypad interactions for additional sound effects.
  - **Volume Changer:** Monitors potentiometer readings to adjust volume on-the-fly.

- **Node.js Web Control**
  - Remote control via a web interface hosted on a Node.js server.
  - Utilizes Socket.IO for real-time communication, allowing users to pause, skip, and manage tracks remotely.

---

## 📁 Project Structure

```plaintext
embedded-audio-player/
├── README.md
├── .gitignore
├── firmware/
│   ├── Makefile
│   ├── include/
│   │   ├── accelerometer_listener/
│   │   │   └── accelerometer_listener.h
│   │   ├── audio_parsers/
│   │   │   └── wav_parser.h
│   │   ├── drivers/
│   │   │   ├── accelerometer.h
│   │   │   ├── joystick.h
│   │   │   ├── neo_trellis.h
│   │   │   ├── potentiometer.h
│   │   │   ├── see_saw.h
│   │   │   └── seg_display.h
│   │   ├── joystick_listener/
│   │   │   └── joystick_listener.h
│   │   ├── launch_pad/
│   │   │   └── launch_pad.h
│   │   ├── segment_display_driver/
│   │   │   └── seg_display_driver.h
│   │   ├── shutdown/
│   │   │   └── shutdown.h
│   │   ├── utils/
│   │   │   └── sleep_milliseconds.h
│   │   ├── volume_changer/
│   │   │   └── volume_changer.h
│   │   └── wave_audio_player/
│   │       └── wave_audio_player.h
│   └── src/
│       ├── accelerometer_listener/
│       │   └── accelerometer_listener.c
│       ├── audio_parsers/
│       │   └── wav_parser.c
│       ├── drivers/
│       │   ├── accelerometer.c
│       │   ├── joystick.c
│       │   ├── neo_trellis.c
│       │   ├── potentiometer.c
│       │   ├── see_saw.c
│       │   └── seg_display.c
│       ├── joystick_listener/
│       │   └── joystick_listener.c
│       ├── launch_pad/
│       │   └── launch_pad.c
│       ├── segment_display_driver/
│       │   └── seg_display_driver.c
│       ├── shutdown/
│       │   └── shutdown.c
│       ├── utils/
│       │   └── sleep_milliseconds.c
│       ├── volume_changer/
│       │   └── volume_changer.c
│       ├── wave_audio_player/
│       │   └── wave_audio_player.c
│       └── main.c
├── server/
│   ├── package.json
│   ├── package-lock.json
│   ├── .cproject
│   ├── .project
│   ├── .settings/
│   │   └── language.settings.xml
│   ├── lib/
│   │   └── udp_server.js
│   ├── public/
│   │   ├── index.html
│   │   ├── javascripts/
│   │   │   └── udp_ui.js
│   │   └── stylesheets/
│   │       └── style.css
│   └── server.js
└── assets/
    ├── music/
    │   ├── track1.wav
    │   ├── track2.wav
    │   └── ...
    └── sound_effects/
        ├── effect1.wav
        ├── effect2.wav
        └── ...
```
## 📂 Key Directories

    firmware/
        Description: Contains all embedded C/C++ code and the Makefile.
        Subdirectories:
            include/: Header files organized by module (e.g., drivers, listeners, utilities).
            src/: Corresponding .c source files.
            main.c: Entry point for the firmware application.

    server/
        Description: Hosts the Node.js application providing a web-based interface for controlling the audio player.
        Subdirectories:
            public/: Front-end assets (HTML, CSS, JavaScript).
            lib/: Server-side libraries and scripts.
        Key Files:
            server.js: Main server script.
            package.json & package-lock.json: Node.js dependencies and configurations.

    assets/
        Description: Stores all audio files used by the player.
        Subdirectories:
            music/: Primary .wav files for playback.
            sound_effects/: Short .wav samples triggered by NeoTrellis button presses.

## 🛠 Hardware Requirements

To fully utilize the Embedded Audio Player, ensure you have the following hardware components:

    BeagleBone Green (or similar BeagleBone variant)
    NeoTrellis 4×4 RGB Matrix (I2C-based) + Adafruit seesaw chip
    Joystick wired to BeagleBone GPIO
    Analog Potentiometer for volume control via ADC pin
    Accelerometer (I2C-based) for gesture detection
    14-Segment Display (I2C or GPIO-based) for displaying track progress/status
    Audio Output (e.g., 3.5 mm headphone jack, external amplifier/speaker)

## 🧩 Installation
### ⚙️ Firmware Setup

    Clone the Repository:

git clone https://github.com/abay-kulamkadyr/embedded_system_audio_player.git
cd embedded-audio-player/firmware

Build the Firmware:

Ensure you have the necessary cross-compilation tools installed (e.g., arm-linux-gnueabihf-gcc).

make

This will compile the firmware and generate the audio_project executable in the deployment directory.

Deploy to BeagleBone:

    Copy the Binary:

cp audio_project /path/to/beaglebone/deployment/directory/

Install ALSA Libraries on BeagleBone:

SSH into your BeagleBone and run:

        sudo apt-get update
        sudo apt-get install libasound2

    Add Audio Files:

    Place your .wav files in the assets/music/ directory and any desired sound effects in assets/sound_effects/.

### 🌐 Node.js Server Setup

    Navigate to the Server Directory:

cd ../server

Install Dependencies:

npm install

Start the Server:

node server.js

Alternatively, you can use:

    npm start

    Access the Web Interface:

    Open a browser and navigate to http://<BEAGLEBONE_IP>:8088 to interact with the audio player remotely.

## 🎮 Usage

    Power On the BeagleBone:

    Ensure all hardware components are connected and powered.

    Run the Firmware:

    SSH into your BeagleBone and execute:

    ./audio_project

    The firmware initializes all modules and begins playback based on the provided .wav files.

    Control via Hardware:
        Joystick: Navigate through tracks.
        Accelerometer: Tilt to skip tracks forward or backward.
        NeoTrellis: Trigger sound effects and control LEDs.
        Potentiometer: Adjust the system volume.
        14-Segment Display: Monitor track progress and status.

    Control via Web Interface:

    Use the Node.js web interface to remotely pause, skip, rewind, and manage tracks.

## 🔄 Usage Flow

    Firmware Execution:
        Initializes all hardware modules.
        Starts ALSA playback threads.
        Enters a blocking state, awaiting user input or shutdown signals.

    Web Interface Interaction:
        Users interact with a web-based UI to send commands (pause, skip, etc.).
        Commands are transmitted via Socket.IO to the server.
        The server relays these commands as UDP packets to the firmware application.

    Audio Playback & Mixing:
        The firmware streams .wav files to the audio output using ALSA.
        Concurrently, user-triggered sound effects from NeoTrellis are mixed in real-time.
        Volume adjustments and track navigation are handled dynamically based on user input.

## 🛠 Known Limitations & Future Improvements

    Buffer Underruns:
        Issue: Skipping or stuttering may occur under high CPU load.
        Solution: Adjust ALSA buffer sizes or optimize thread handling.

    Large File Support:
        Issue: Limited memory for large .wav files.
        Solution: Implement streaming from disk instead of loading entire files into memory.
