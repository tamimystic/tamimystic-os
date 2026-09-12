#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace TamimysticOS {

enum class VoiceCommand {
    NONE = 0,
    WAKE_HEY_TAMIMYSTIC,
    DRIVE_FORWARD,
    DRIVE_BACKWARD,
    TURN_LEFT,
    TURN_RIGHT,
    STOP,
    ARM_HOME,
    GRAB_OBJECT,
    STATUS_REPORT
};

inline const char* voiceCommandToString(VoiceCommand cmd) {
    switch (cmd) {
        case VoiceCommand::WAKE_HEY_TAMIMYSTIC: return "Hey Tamimystic";
        case VoiceCommand::DRIVE_FORWARD:       return "Drive Forward";
        case VoiceCommand::DRIVE_BACKWARD:      return "Drive Backward";
        case VoiceCommand::TURN_LEFT:           return "Turn Left";
        case VoiceCommand::TURN_RIGHT:          return "Turn Right";
        case VoiceCommand::STOP:                return "Emergency Stop";
        case VoiceCommand::ARM_HOME:            return "Arm Home";
        case VoiceCommand::GRAB_OBJECT:         return "Grab Object";
        case VoiceCommand::STATUS_REPORT:       return "Status Report";
        default:                                return "None";
    }
}

inline const char* voiceCommandToKeyword(VoiceCommand cmd) {
    switch (cmd) {
        case VoiceCommand::WAKE_HEY_TAMIMYSTIC: return "hey_tamimystic";
        case VoiceCommand::DRIVE_FORWARD:       return "forward";
        case VoiceCommand::DRIVE_BACKWARD:      return "backward";
        case VoiceCommand::TURN_LEFT:           return "left";
        case VoiceCommand::TURN_RIGHT:          return "right";
        case VoiceCommand::STOP:                return "stop";
        case VoiceCommand::ARM_HOME:            return "arm_home";
        case VoiceCommand::GRAB_OBJECT:         return "grab";
        case VoiceCommand::STATUS_REPORT:       return "status";
        default:                                return "none";
    }
}

enum class AudioState {
    IDLE = 0,
    LISTENING_KWS,
    PROCESSING,
    SPEAKING
};

inline const char* audioStateToString(AudioState state) {
    switch (state) {
        case AudioState::LISTENING_KWS: return "LISTENING";
        case AudioState::PROCESSING:    return "PROCESSING";
        case AudioState::SPEAKING:      return "SPEAKING";
        default:                        return "IDLE";
    }
}

struct AudioConfig {
    int pin_bclk = 41;
    int pin_ws = 42;
    int pin_din = 40;   // Digital Mic INMP441 Data Input
    int pin_dout = 39;  // I2S DAC MAX98357A / Speaker Data Output
    uint32_t sample_rate = 16000;
    uint8_t volume = 80;
    bool kws_active = true;
};

struct AudioStatus {
    AudioState state = AudioState::IDLE;
    VoiceCommand last_command = VoiceCommand::NONE;
    std::string last_command_str = "None";
    float confidence = 0.0f;
    float energy_level_db = -45.0f;
    bool is_speaking = false;
    bool is_listening = true;
    uint8_t volume = 80;
    uint32_t processed_frames = 0;
};

} // namespace TamimysticOS
