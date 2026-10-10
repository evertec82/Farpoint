// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <cmath>
#include <optional>
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

// Relate two tangent-to-UV projections without changing either eye's projection.
inline std::optional<std::array<float, 4>> OverlaySourceUv(std::array<float, 4> base,
                                                         std::array<float, 4> overlay) {
    for (unsigned i = 0; i < 4; ++i) {
        if (!std::isfinite(base[i]) || !std::isfinite(overlay[i])) {
            return std::nullopt;
        }
    }
    if (base[0] <= 0 || base[1] <= 0 || overlay[0] <= 0 || overlay[1] <= 0) {
        return std::nullopt;
    }
    const float sx = overlay[0] / base[0];
    const float sy = overlay[1] / base[1];
    const std::array result{sx, sy, overlay[2] - base[2] * sx, overlay[3] - base[3] * sy};
    for (float value : result) {
        if (!std::isfinite(value)) {
            return std::nullopt;
        }
    }
    return result;
}
} // namespace Core::Vr
