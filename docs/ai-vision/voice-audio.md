# Voice Control and Audio Edge AI

Tamimystic OS features an integrated **Edge Audio DSP and Voice Control Engine** (`os_audio`). Operating over the I2S digital audio bus, the system captures 16 kHz 16-bit PCM audio, computes real-time Mel-Frequency Cepstral Coefficients (MFCC), and runs an on-device Keyword Spotting (KWS) deep learning network to execute voice commands without internet connectivity.

---

## Hardware Architecture: Digital I2S Bus

The audio subsystem interfaces with an I2S MEMS microphone (Knowles INMP441) and an I2S Class-D amplifier (Maxim Integrated MAX98357A):

```mermaid
graph LR
    subgraph AudioTransceivers["Audio Peripherals"]
        INMP441["INMP441 Digital MEMS Mic (16 kHz / 24-bit)"]
        MAX98357A["MAX98357A 3.2W I2S Class-D DAC Amp"]
    end

    subgraph ESP32Target["ESP32-S3 SoC"]
        I2S0["I2S0 Hardware Peripheral Controller"]
        AudioDMA["DMA Audio Ring Buffers (Ping-Pong in PSRAM)"]
        DSPPipeline["MFCC Feature Extraction Pipeline (FFT & Mel Banks)"]
        KWSEngine["1D-CNN / GRU Keyword Spotter (Core 1)"]
        AudioSynth["WAV / PCM Voice Synthesizer & LittleFS Player"]
    end

    INMP441 -->|I2S SD_IN (GPIO 39)| I2S0
    I2S0 -->|I2S SD_OUT (GPIO 40)| MAX98357A
    I2S0 <--> AudioDMA
    AudioDMA --> DSPPipeline
    DSPPipeline --> KWSEngine
    KWSEngine -->|Trigger Action| Motion["1000Hz Motion & Arm Engine"]
    AudioSynth --> AudioDMA
```

### I2S Physical Pin Mappings:
- **`I2S_BCLK`**: Bit Clock (Default: **GPIO 41**)
- **`I2S_WS`**: Word Select / LRCLK (Default: **GPIO 42**)
- **`I2S_DIN`**: Digital Audio Input from Microphone (Default: **GPIO 39**)
- **`I2S_DOUT`**: Digital Audio Output to Speaker Amplifier (Default: **GPIO 40**)

---

## Mel-Frequency Cepstral Coefficients (MFCC) Pipeline

To convert raw 16 kHz acoustic waveforms into compact spectral representations for neural classification, the DSP engine processes continuous $32\text{ ms}$ windows ($512\text{ samples}$) with a $16\text{ ms}$ hop step ($256\text{ samples}$):

$$\text{Raw PCM Frame } x[n] \xrightarrow{\text{Hamming Window}} w[n] \cdot x[n] \xrightarrow{\text{512-Point FFT}} |X[k]| \xrightarrow{\text{Mel Filterbank}} M[m] \xrightarrow{\text{Log + DCT}} \text{MFCC}[c]$$

1. **Pre-Emphasis**: Boosts high frequencies ($y[n] = x[n] - 0.97 \cdot x[n-1]$).
2. **Hamming Windowing**: Prevents spectral leakage:
   $$w[n] = 0.54 - 0.46 \cdot \cos\left(\frac{2\pi n}{N - 1}\right)$$
3. **512-Point Fast Fourier Transform (FFT)**: Computes power spectrum $|X(k)|^2$.
4. **40 Triangular Mel-Scale Filterbanks**: Maps linear frequencies to non-linear human auditory scale:
   $$m = 2595 \cdot \log_{10}\left(1 + \frac{f}{700}\right)$$
5. **Discrete Cosine Transform (DCT-II)**: Compresses the log Mel energies into 13 to 20 MFCC coefficients per frame.

---

## Keyword Spotting (KWS) Neural Network

The KWS classifier is a 1D Temporal Depthwise-Separable Convolutional Neural Network (1D-CNN) running on Core 1:

| Command Phrase | Triggered Action | Confidence Threshold |
|---|---|---|
| **"Hey Mystic"** | System Wake-Word / Arm Audio Listening State | $0.85$ |
| **"Forward"** | Drive Rover Forward ($v_x = 0.4\text{ m/s}$) | $0.80$ |
| **"Backward"** | Drive Rover Reverse ($v_x = -0.3\text{ m/s}$) | $0.80$ |
| **"Stop"** | Immediate Motion & Arm E-STOP | $0.75$ |
| **"Turn Left"** | Rotate Left ($\omega_z = +1.0\text{ rad/s}$) | $0.80$ |
| **"Turn Right"** | Rotate Right ($\omega_z = -1.0\text{ rad/s}$) | $0.80$ |
| **"Pick Object"** | Execute 6-DOF Gripper Grab Trajectory | $0.85$ |
| **"Status"** | Play Audio Battery and Network Diagnostic Summary | $0.80$ |

---

## Audio Voice Feedback Synthesizer

Tamimystic OS can playback pre-recorded WAV voice alerts stored in the LittleFS Virtual File System:

```bash
# Play audio file from LittleFS
tamimystic> audio play /storage/audio/startup.wav
```

---

## MicroPython Audio API

```python
import tamimystic
import time

# Start continuous keyword spotting listening task
tamimystic.audio.start_kws()

# Play audio alert through I2S speaker
tamimystic.audio.play_wav("/storage/audio/ready.wav")

print("Voice control engine listening for 'Hey Mystic'...")

while True:
    keyword = tamimystic.audio.get_last_keyword()
    if keyword:
        print(f"Detected Voice Command: {keyword['command']} (Score: {keyword['score']:.2f})")
        if keyword['command'] == "forward":
            tamimystic.motion.drive(0.4, 0.0)
        elif keyword['command'] == "stop":
            tamimystic.motion.stop()
    time.sleep(0.05)
```

---

## Serial CLI and REST API Reference

### Serial CLI:
```bash
# Start voice keyword spotting daemon
tamimystic> audio kws start

# Stop audio engine
tamimystic> audio kws stop

# Play a test tone (frequency in Hz, duration in ms)
tamimystic> audio tone 1000 500

# Query audio DSP status
tamimystic> audio status
```

### HTTP REST API:
```bash
# Start KWS
POST /api/audio/kws/start

# Play audio file
POST /api/audio/play?file=/storage/audio/alert.wav

# Query audio recognition state
GET /api/audio/status
```
