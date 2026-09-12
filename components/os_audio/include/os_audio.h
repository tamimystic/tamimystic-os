#pragma once

#include "os_audio_types.h"
#include <string>
#include <vector>
#include <deque>
#include <mutex>

namespace TamimysticOS {

class AudioEngine {
public:
    static AudioEngine& getInstance();

    // Initialize I2S audio interfaces and start FreeRTOS Core 1 audio task
    void init(const AudioConfig& config = {});

    // Keyword Spotting (KWS) control
    void setKwsEnabled(bool enabled);
    bool isKwsEnabled() const { return config.kws_active; }

    // Volume control (0 - 100)
    void setVolume(uint8_t volume);
    uint8_t getVolume() const { return config.volume; }

    // Status and Telemetry
    AudioStatus getStatus() const;
    std::string getStatusJson() const;
    std::string getWaveformJson() const;

    // Speech Synthesis & Sound Generation
    void speak(const std::string& phrase);
    void speakCommandResponse(VoiceCommand cmd);
    void playTone(uint16_t freq_hz, uint16_t duration_ms);
    void playBeepPattern(int pattern_id);

    // Simulated / Injected Voice Command execution
    VoiceCommand triggerKwsTest(const std::string& command_name);
    void executeVoiceCommand(VoiceCommand cmd);

    // Audio Frame Processing Loop (Runs on Core 1)
    void processAudioTask();

private:
    AudioEngine() = default;
    ~AudioEngine() = default;

    void generatePcmTone(uint16_t freq_hz, uint16_t duration_ms, std::vector<int16_t>& pcm_out);
    void synthesizeSpeechPcm(const std::string& phrase, std::vector<int16_t>& pcm_out);
    VoiceCommand classifyAudioFrame(const int16_t* pcm, size_t count, float& out_confidence);

    AudioConfig config;
    AudioStatus status;
    mutable std::mutex audio_mutex;

    std::deque<float> live_waveform; // Recent 32 audio energy samples for visualizer
    uint32_t simulated_frame_counter = 0;
};

} // namespace TamimysticOS
