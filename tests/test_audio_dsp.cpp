#include "test_framework.h"
#include "os_audio.h"

using namespace TamimysticOS;

void test_audio_engine_controls() {
    auto& audio = AudioEngine::getInstance();
    audio.init();

    // Volume configuration test
    audio.setVolume(85);
    TEST_ASSERT_EQ((int)audio.getVolume(), 85, "Volume should be set to 85%");

    audio.setVolume(120); // Clamped to 100
    TEST_ASSERT_EQ((int)audio.getVolume(), 100, "Volume should clamp to 100%");

    // KWS enablement
    audio.setKwsEnabled(true);
    TEST_ASSERT(audio.isKwsEnabled(), "KWS should be enabled");

    audio.setKwsEnabled(false);
    TEST_ASSERT(!audio.isKwsEnabled(), "KWS should be disabled");
}

void test_voice_command_trigger() {
    auto& audio = AudioEngine::getInstance();

    VoiceCommand cmd1 = audio.triggerKwsTest("forward");
    TEST_ASSERT_EQ((int)cmd1, (int)VoiceCommand::DRIVE_FORWARD, "Trigger 'forward' command");

    VoiceCommand cmd2 = audio.triggerKwsTest("stop");
    TEST_ASSERT_EQ((int)cmd2, (int)VoiceCommand::STOP, "Trigger 'stop' command");

    VoiceCommand cmd3 = audio.triggerKwsTest("hey");
    TEST_ASSERT_EQ((int)cmd3, (int)VoiceCommand::WAKE_HEY_TAMIMYSTIC, "Trigger 'hey' wake word");
}

void run_audio_dsp_test_suite() {
    RUN_TEST_SUITE("Audio DSP Engine & Volume Clamping", test_audio_engine_controls);
    RUN_TEST_SUITE("Keyword Spotting Neural Command Parsing", test_voice_command_trigger);
}
