# 🇮🇳 Offline Hindi Voice Assistant  
### Bharat AI-SOC Student Challenge – Problem Statement 1

---

## Problem Statement

Develop an offline, privacy-preserving Hindi voice assistant that runs completely on edge hardware without any cloud dependency.

---

## Project Overview

This project implements a fully offline Hindi voice assistant capable of:

- Hindi Speech-to-Text
- Command Processing
- Hindi Text-to-Speech
- 100% Privacy (fully offline)

The assistant runs entirely on Raspberry Pi 4 using ARM architecture.

---

##  System Architecture

User Speech  
↓  
Speech-to-Text Engine  
↓  
NLP & Logics  
↓  
Response Generation  
↓  
Text-to-Speech Engine  
↓  
Audio Output  

---

##  Technologies Used

- C++, Python  
- Raspberry Pi 4 
- ARM Processor  
- VOSK (STT) 
- DSP powered eSpeak-NG (TTS)  

---



##  How to Run

### System Requirements

- Raspberry Pi 4  
- Raspberry Pi OS Lite (Bullseye)  
- INMP441 I2S Microphone  
- Speaker / Audio Output  
- Internet connection (only for initial setup & model download)



### 1️ Update the System

```bash
sudo apt update
sudo apt upgrade -y
```



### 2️ Install System Dependencies

```bash
sudo apt install -y python3-pip python3-dev espeak-ng alsa-utils portaudio19-dev sqlite3
```

These packages are required for:
- Python environment setup
- ALSA audio handling
- eSpeak-NG text-to-speech
- SQLite database management



### 3️ Install Python Libraries

```bash
pip3 install -r requirements.txt
```

`requirements.txt` includes:

- vosk  
- numpy  



### 4️ Download VOSK Hindi Model

Download the Hindi speech recognition model:

**vosk-small-model-hi**

From:
https://alphacephei.com/vosk/models

After downloading, extract the model folder into your project directory:

```bash
cd offline-hindi-voice-assistant-leo
mkdir -p model
```

Place the extracted VOSK model folder inside:

```
offline-hindi-voice-assistant-leo/model/
```

Make sure the model path inside `main.py` matches the extracted folder name.



### 5️ Verify Audio Configuration (ALSA)

Check microphone detection:

```bash
arecord -l
```

Test recording:

```bash
arecord test.wav
aplay test.wav
```

If you hear playback successfully, audio is configured correctly.


### 6️ Run the Voice Assistant

```bash
python3 main.py
```

The assistant will:
- Capture Hindi speech input  
- Convert speech to text using VOSK  
- Process intent and long-term memory using SQLite + Qwen  
- Generate spoken response using eSpeak-NG  



##  Database (SQLite)

- The SQLite database file (`assistant.db`) is automatically created if not present.  
- Stores command history and long-term conversational memory.  
- Fully local storage — no external database required.  



## System Architecture


User Speech
    ↓
VOSK (Hindi STT)
    ↓
Intent Processing + SQLite Memory
    ↓
Qwen Reasoning Engine
    ↓
eSpeak-NG (TTS)
    ↓
Audio Output
## To make it fully offline assitant 
cp primus.service /home/pi/primus/ml/
cp install_service.sh /home/pi/primus/ml/
cd /home/pi/primus/ml
sudo bash install_service.sh


## Use full commands after install
  journalctl -u primus -f

# Stop
sudo systemctl stop primus

# Restart
sudo systemctl restart primus

# Disable autostart
sudo systemctl disable primus

# Check status
sudo systemctl status primus
## Wrap the entire system with the files 
  Primus
  Install service



##  Project Presentation

 [View Presentation (Google Drive)](https://docs.google.com/presentation/d/1L_QAvwp2dqZpFrOXG62mxtLjEKvuC7DD/edit?usp=drive_link&ouid=118051758676508864614&rtpof=true&sd=true)

