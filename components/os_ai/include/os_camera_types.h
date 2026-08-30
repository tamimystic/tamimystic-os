#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

namespace TamimysticOS {

enum class FrameFormat {
    FORMAT_JPEG = 0,
    FORMAT_RGB565,
    FORMAT_GRAYSCALE
};

enum class FrameResolution {
    RES_QQVGA = 0, // 160x120
    RES_QVGA,      // 320x240
    RES_VGA,       // 640x480
    RES_SVGA,      // 800x600
    RES_HD         // 1280x720
};

inline void getResolutionDimensions(FrameResolution res, int& w, int& h) {
    switch (res) {
        case FrameResolution::RES_QQVGA: w = 160;  h = 120; break;
        case FrameResolution::RES_QVGA:  w = 320;  h = 240; break;
        case FrameResolution::RES_VGA:   w = 640;  h = 480; break;
        case FrameResolution::RES_SVGA:  w = 800;  h = 600; break;
        case FrameResolution::RES_HD:    w = 1280; h = 720; break;
        default:                         w = 320;  h = 240; break;
    }
}

struct CameraFrame {
    uint8_t* buf = nullptr;
    size_t len = 0;
    int width = 320;
    int height = 240;
    FrameFormat format = FrameFormat::FORMAT_JPEG;
    uint32_t timestamp_ms = 0;
};

} // namespace TamimysticOS
