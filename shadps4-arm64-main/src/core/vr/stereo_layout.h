// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
namespace Core::Vr {
// Scale XY and offset XY in the source image, for horizontal side-by-side stereo.
constexpr std::array<float, 4> EyeSourceUv(bool packed, unsigned eye) {
    return packed ? std::array<float, 4>{0.5f, 1.0f, eye * 0.5f, 0.0f}
                  : std::array<float, 4>{1.0f, 1.0f, 0.0f, 0.0f};
}
// Transform tangent-to-UV coefficients from the whole image into the left eye's half.
constexpr std::array<float, 4> LeftEyeProjectionUv(std::array<float, 4> uv, bool packed) {
    if (packed) {
        uv[0] *= 2.0f;
        uv[2] *= 2.0f;
    }
    return uv;
}
} // namespace Core::Vr
