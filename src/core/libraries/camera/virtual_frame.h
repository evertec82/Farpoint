// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Libraries::Camera {
// Base: 0=YUV422, 1=RAW16, 2=RAW8. Scaled: 0=YUV422, 3=Y16, 4=Y8.
inline std::size_t VirtualFrameBytes(std::uint32_t width, std::uint32_t height,
                                     unsigned format) {
    if (format > 4) {
        return 0;
    }
    return std::size_t{width} * height * ((format == 2 || format == 4) ? 1 : 2);
}

inline std::vector<std::uint8_t> VirtualBlackYuy2(std::size_t pixels) {
    std::vector<std::uint8_t> bytes(pixels * 2);
    // Limited-range YUY2: black luma with neutral U/V, not zero chroma (green).
    for (std::size_t i = 0; i < bytes.size(); i += 2) {
        bytes[i] = 16;
        bytes[i + 1] = 128;
    }
    return bytes;
}
} // namespace Libraries::Camera
