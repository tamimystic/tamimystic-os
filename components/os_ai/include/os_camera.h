#pragma once

#include "os_camera_types.h"
#include <vector>

namespace TamimysticOS {

class CameraManager {
public:
    static CameraManager& getInstance();

    // Initialize camera hardware or native frame simulator
    bool init(FrameResolution resolution = FrameResolution::RES_QVGA, 
              FrameFormat format = FrameFormat::FORMAT_JPEG);

    // Acquire a new camera frame
    CameraFrame* getFrame();

    // Release frame back to memory pool / PSRAM
    void returnFrame(CameraFrame* frame);

    // Camera state
    bool isInitialized() const;
    FrameResolution getResolution() const;
    FrameFormat getFormat() const;

private:
    CameraManager() = default;
    ~CameraManager() = default;

    void generateNativeMockFrame();

    bool initialized = false;
    FrameResolution current_resolution = FrameResolution::RES_QVGA;
    FrameFormat current_format = FrameFormat::FORMAT_JPEG;
    
    // Native mock frame buffer
    CameraFrame native_frame;
    std::vector<uint8_t> mock_jpeg_buffer;
    uint32_t frame_counter = 0;
};

} // namespace TamimysticOS
