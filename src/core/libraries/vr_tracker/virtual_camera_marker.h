// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <cmath>
#include <optional>

namespace Libraries::VrTracker {
struct VirtualCameraMarker {
    float x;
    float y;
};

// The virtual camera is level, at the tracking origin, looking along +Z.
// Use a 90-degree horizontal field of view and the virtual frame's 1280:800
// aspect ratio. This is a host-defined camera, not physical PS Camera optics.
// Its marker represents the tracked headset centre, not a fabricated LED array.
inline std::optional<VirtualCameraMarker> ProjectVirtualCameraMarker(float x, float y, float z) {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || z <= 0.01f) {
        return std::nullopt;
    }
    const VirtualCameraMarker marker{0.5f + x / (2.0f * z), 0.5f - y / (1.25f * z)};
    if (marker.x < 0.0f || marker.x > 1.0f || marker.y < 0.0f || marker.y > 1.0f) {
        return std::nullopt;
    }
    return marker;
}
} // namespace Libraries::VrTracker
