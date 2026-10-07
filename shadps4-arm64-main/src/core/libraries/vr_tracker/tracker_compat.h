// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstddef>
#include <cstring>
#include "core/libraries/vr_tracker/vr_tracker.h"
namespace Libraries::VrTracker {
// Farpoint 1.00's observed 144-byte SDK layout puts calibration at 0x20,
// memory descriptors at 0x40 and compute queues at 0x70. The 128-byte ABI
// puts an extra CPU mask before calibration. Its trailing 24 bytes are not used by HLE.
inline bool DecodeFarpointTrackerInit144(const void* data, std::size_t size,
                                         OrbisVrTrackerInitParam& out) {
    static_assert(sizeof(out) == 128);
    static_assert(offsetof(OrbisVrTrackerInitParam, calibration_settings) == 0x28);
    static_assert(offsetof(OrbisVrTrackerInitParam, direct_memory_onion) == 0x48);
    static_assert(offsetof(OrbisVrTrackerInitParam, gpu_pipe_id) == 0x78);
    if (!data || size != 144)
        return false;
    out = {};
    auto* dest = reinterpret_cast<unsigned char*>(&out);
    const auto* src = static_cast<const unsigned char*>(data);
    std::memcpy(dest, src, 0x20);
    std::memcpy(dest + 0x28, src + 0x20, 0x20);
    std::memcpy(dest + 0x48, src + 0x40, 0x38);
    out.size = sizeof(out);
    return true;
}
} // namespace Libraries::VrTracker
