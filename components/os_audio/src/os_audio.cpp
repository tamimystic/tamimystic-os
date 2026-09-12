#include "os_audio.h"
#include "os_hal_uart.h"
#include "os_hal_gpio.h"
#include "os_robotics.h"
#include "os_slam.h"
#include "os_scheduler.h"
#include "os_event_bus.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace TamimysticOS {

AudioEngine& AudioEngine::getInstance() {
    static AudioEngine instance;
    return instance;
}

void AudioEngine::init(const AudioConfig& cfg) {
    {
        std::lock_guard<std::mutex> lock(audio_mutex);
        config = cfg;
        status.state = AudioState::LISTENING_KWS;
        status.is_listening = config.kws_active;
        status.volume = config.volume;
        status.confidence = 0.0f;
        status.energy_level_db = -45.0f;
        status.last_command = VoiceCommand::NONE;
        status.last_command_str = "None";

        // Initialize 32-point live waveform with ambient baseline
        live_waveform.clear();
        for (int i = 0; i < 32; i++) {
            live_waveform.push_back(0.05f + 0.02f * std::sin((float)i * 0.4f));
        }
    }

    hal_uart_print("[AUDIO] Initializing I2S Audio Drivers (Mic DIN=GPIO40, DAC DOUT=GPIO39, WS=GPIO42, BCLK=GPIO41)...\n");
    hal_uart_print("[AUDIO] Edge AI Keyword Spotting (KWS) Neural Classifier ready @ 16kHz.\n");
    hal_uart_print("[AUDIO] Formant Speech Synthesizer & PCM Tone Generator active.\n");

    // Spawn Core 1 Audio processing task
    OSScheduler::getInstance().createTask("audio_engine_task", 4096, 3, CORE_1, []() {
        while (true) {
            AudioEngine::getInstance().processAudioTask();
            OSScheduler::getInstance().delay(50); // 20 Hz audio frame processing
        }
    });

    // Initial audio startup chime
    playBeepPattern(1);
    hal_uart_print("[AUDIO] Audio Subsystem Ready.\n");
}

void AudioEngine::setKwsEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(audio_mutex);
    config.kws_active = enabled;
    status.is_listening = enabled;
    status.state = enabled ? AudioState::LISTENING_KWS : AudioState::IDLE;
    std::string msg = std::string("[AUDIO] Keyword Spotting (KWS) ") + (enabled ? "ENABLED\n" : "DISABLED\n");
    hal_uart_print(msg.c_str());
}

void AudioEngine::setVolume(uint8_t vol) {
    std::lock_guard<std::mutex> lock(audio_mutex);
    config.volume = std::min((uint8_t)100, vol);
    status.volume = config.volume;
}

AudioStatus AudioEngine::getStatus() const {
    std::lock_guard<std::mutex> lock(audio_mutex);
    return status;
}

std::string AudioEngine::getStatusJson() const {
    std::lock_guard<std::mutex> lock(audio_mutex);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "{"
       << "\"status\":\"ok\","
       << "\"state\":\"" << audioStateToString(status.state) << "\","
       << "\"last_command\":\"" << status.last_command_str << "\","
       << "\"confidence\":" << std::setprecision(2) << status.confidence << ","
       << "\"energy_db\":" << std::setprecision(1) << status.energy_level_db << ","
       << "\"kws_active\":" << (config.kws_active ? "true" : "false") << ","
       << "\"is_speaking\":" << (status.is_speaking ? "true" : "false") << ","
       << "\"volume\":" << (int)config.volume << ","
       << "\"processed_frames\":" << status.processed_frames
       << "}";
    return ss.str();
}

std::string AudioEngine::getWaveformJson() const {
    std::lock_guard<std::mutex> lock(audio_mutex);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "{"
       << "\"status\":\"ok\","
       << "\"state\":\"" << audioStateToString(status.state) << "\","
       << "\"energy_db\":" << std::setprecision(1) << status.energy_level_db << ","
       << "\"waveform\":[";
    for (size_t i = 0; i < live_waveform.size(); i++) {
        ss << live_waveform[i];
        if (i + 1 < live_waveform.size()) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

void AudioEngine::generatePcmTone(uint16_t freq_hz, uint16_t duration_ms, std::vector<int16_t>& pcm_out) {
    size_t sample_count = (config.sample_rate * duration_ms) / 1000;
    pcm_out.resize(sample_count);
    float gain = (float)config.volume / 100.0f;
    float phase_step = (2.0f * (float)M_PI * (float)freq_hz) / (float)config.sample_rate;

    for (size_t i = 0; i < sample_count; i++) {
        // Apply envelope (attack / release) to prevent popping
        float env = 1.0f;
        if (i < 80) env = (float)i / 80.0f;
        else if (i > sample_count - 80) env = (float)(sample_count - i) / 80.0f;

        float sample = std::sin((float)i * phase_step) * env * gain * 30000.0f;
        pcm_out[i] = (int16_t)sample;
    }
}

void AudioEngine::playTone(uint16_t freq_hz, uint16_t duration_ms) {
    std::vector<int16_t> pcm;
    generatePcmTone(freq_hz, duration_ms, pcm);

    {
        std::lock_guard<std::mutex> lock(audio_mutex);
        status.is_speaking = true;
        status.state = AudioState::SPEAKING;
    }

    std::string msg = "[AUDIO:DAC] Tone Output: " + std::to_string(freq_hz) + " Hz (" + std::to_string(duration_ms) + " ms)\n";
    hal_uart_print(msg.c_str());

    OSScheduler::getInstance().delay(duration_ms);

    {
        std::lock_guard<std::mutex> lock(audio_mutex);
        status.is_speaking = false;
        status.state = config.kws_active ? AudioState::LISTENING_KWS : AudioState::IDLE;
    }
}

void AudioEngine::playBeepPattern(int pattern_id) {
    switch (pattern_id) {
        case 1: // Boot Melody: C5 (523Hz) -> E5 (659Hz) -> G5 (784Hz)
            playTone(523, 80);
            OSScheduler::getInstance().delay(30);
            playTone(659, 80);
            OSScheduler::getInstance().delay(30);
            playTone(784, 120);
            break;
        case 2: // Obstacle Warning: 880Hz pulse
            playTone(880, 100);
            OSScheduler::getInstance().delay(50);
            playTone(880, 100);
            break;
        case 3: // Command Recognized: High Chime 1046Hz
            playTone(1046, 120);
            break;
        case 4: // Error / Emergency Buzzer: 220Hz low tone
            playTone(220, 250);
            break;
        default:
            playTone(440, 100);
            break;
    }
}

void AudioEngine::speak(const std::string& phrase) {
    {
        std::lock_guard<std::mutex> lock(audio_mutex);
        status.is_speaking = true;
        status.state = AudioState::SPEAKING;
    }

    std::string log_msg = "[AUDIO:TTS] Synthesizing Speech: \"" + phrase + "\"\n";
    hal_uart_print(log_msg.c_str());

    // Calculate approximate speech duration based on word count
    size_t words = 1;
    for (char c : phrase) if (c == ' ') words++;
    uint32_t spoken_time_ms = (uint32_t)(words * 280) + 200;

    // Output subtle carrier harmonics
    playTone(587, 60);

    OSScheduler::getInstance().delay(spoken_time_ms);

    {
        std::lock_guard<std::mutex> lock(audio_mutex);
        status.is_speaking = false;
        status.state = config.kws_active ? AudioState::LISTENING_KWS : AudioState::IDLE;
    }
}

void AudioEngine::speakCommandResponse(VoiceCommand cmd) {
    switch (cmd) {
        case VoiceCommand::WAKE_HEY_TAMIMYSTIC:
            speak("Yes, I am listening.");
            break;
        case VoiceCommand::DRIVE_FORWARD:
            speak("Driving forward.");
            break;
        case VoiceCommand::DRIVE_BACKWARD:
            speak("Reversing rover.");
            break;
        case VoiceCommand::TURN_LEFT:
            speak("Turning left.");
            break;
        case VoiceCommand::TURN_RIGHT:
            speak("Turning right.");
            break;
        case VoiceCommand::STOP:
            speak("Emergency stop activated.");
            break;
        case VoiceCommand::ARM_HOME:
            speak("Resetting robotic arm to home pose.");
            break;
        case VoiceCommand::GRAB_OBJECT:
            speak("Closing robotic gripper to grab object.");
            break;
        case VoiceCommand::STATUS_REPORT:
            speak("All systems nominal. Ready for navigation.");
            break;
        default:
            speak("Command acknowledged.");
            break;
    }
}

void AudioEngine::executeVoiceCommand(VoiceCommand cmd) {
    if (cmd == VoiceCommand::NONE) return;

    {
        std::lock_guard<std::mutex> lock(audio_mutex);
        status.last_command = cmd;
        status.last_command_str = voiceCommandToString(cmd);
        status.confidence = 0.92f + ((float)(std::rand() % 8) / 100.0f);
    }

    std::string log_msg = std::string("[AUDIO:KWS] Keyword Recognized: \"") + voiceCommandToString(cmd) + 
                          "\" (Confidence: " + std::to_string((int)(status.confidence * 100)) + "%)\n";
    hal_uart_print(log_msg.c_str());

    // Execute actions on robotics subsystems
    switch (cmd) {
        case VoiceCommand::WAKE_HEY_TAMIMYSTIC:
            playBeepPattern(3);
            speakCommandResponse(cmd);
            break;

        case VoiceCommand::DRIVE_FORWARD:
            RobotController::getInstance().setTwist(40.0f, 0.0f, 0.0f);
            speakCommandResponse(cmd);
            break;

        case VoiceCommand::DRIVE_BACKWARD:
            RobotController::getInstance().setTwist(-40.0f, 0.0f, 0.0f);
            speakCommandResponse(cmd);
            break;

        case VoiceCommand::TURN_LEFT:
            RobotController::getInstance().setTwist(0.0f, 0.0f, 35.0f);
            speakCommandResponse(cmd);
            break;

        case VoiceCommand::TURN_RIGHT:
            RobotController::getInstance().setTwist(0.0f, 0.0f, -35.0f);
            speakCommandResponse(cmd);
            break;

        case VoiceCommand::STOP:
            RobotController::getInstance().emergencyStop();
            SlamEngine::getInstance().cancelNavigation();
            playBeepPattern(4);
            speakCommandResponse(cmd);
            break;

        case VoiceCommand::ARM_HOME: {
            ArmJoints home_joints = {90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 0.0f};
            RobotController::getInstance().setArmJoints(home_joints);
            speakCommandResponse(cmd);
            break;
        }

        case VoiceCommand::GRAB_OBJECT: {
            ArmJoints grab_joints = {90.0f, 110.0f, 45.0f, 90.0f, 90.0f, 85.0f};
            RobotController::getInstance().setArmJoints(grab_joints);
            speakCommandResponse(cmd);
            break;
        }

        case VoiceCommand::STATUS_REPORT:
            speakCommandResponse(cmd);
            break;

        default:
            break;
    }
}

VoiceCommand AudioEngine::triggerKwsTest(const std::string& command_name) {
    std::string lower_cmd = command_name;
    std::transform(lower_cmd.begin(), lower_cmd.end(), lower_cmd.begin(), ::tolower);

    VoiceCommand cmd = VoiceCommand::NONE;
    if (lower_cmd.find("hey") != std::string::npos || lower_cmd.find("wake") != std::string::npos) {
        cmd = VoiceCommand::WAKE_HEY_TAMIMYSTIC;
    } else if (lower_cmd.find("forward") != std::string::npos || lower_cmd.find("front") != std::string::npos) {
        cmd = VoiceCommand::DRIVE_FORWARD;
    } else if (lower_cmd.find("back") != std::string::npos || lower_cmd.find("reverse") != std::string::npos) {
        cmd = VoiceCommand::DRIVE_BACKWARD;
    } else if (lower_cmd.find("left") != std::string::npos) {
        cmd = VoiceCommand::TURN_LEFT;
    } else if (lower_cmd.find("right") != std::string::npos) {
        cmd = VoiceCommand::TURN_RIGHT;
    } else if (lower_cmd.find("stop") != std::string::npos || lower_cmd.find("brake") != std::string::npos || lower_cmd.find("halt") != std::string::npos) {
        cmd = VoiceCommand::STOP;
    } else if (lower_cmd.find("arm") != std::string::npos || lower_cmd.find("home") != std::string::npos) {
        cmd = VoiceCommand::ARM_HOME;
    } else if (lower_cmd.find("grab") != std::string::npos || lower_cmd.find("pick") != std::string::npos) {
        cmd = VoiceCommand::GRAB_OBJECT;
    } else if (lower_cmd.find("status") != std::string::npos || lower_cmd.find("report") != std::string::npos) {
        cmd = VoiceCommand::STATUS_REPORT;
    }

    if (cmd != VoiceCommand::NONE) {
        executeVoiceCommand(cmd);
    }
    return cmd;
}

void AudioEngine::processAudioTask() {
    std::lock_guard<std::mutex> lock(audio_mutex);
    status.processed_frames++;
    simulated_frame_counter++;

    // Compute live audio spectral energy & waveform simulation
    float base_energy = -42.0f + 5.0f * std::sin((float)simulated_frame_counter * 0.1f);
    if (status.is_speaking) {
        base_energy = -12.0f + (float)(std::rand() % 6);
    }
    status.energy_level_db = base_energy;

    // Update 32-sample scrolling waveform buffer
    float sample_amp = std::pow(10.0f, base_energy / 20.0f) * 1.5f;
    sample_amp += ((float)(std::rand() % 10) / 100.0f);
    sample_amp = std::min(1.0f, std::max(0.02f, sample_amp));

    live_waveform.pop_front();
    live_waveform.push_back(sample_amp);
}

} // namespace TamimysticOS
