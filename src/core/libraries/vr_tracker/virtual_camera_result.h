// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstddef>
#include "core/libraries/vr_tracker/virtual_camera_marker.h"
#include "core/libraries/vr_tracker/vr_tracker.h"

namespace Libraries::VrTracker {
static_assert(offsetof(OrbisVrTrackerResultData, number_of_led_result) == 0x1d8);
static_assert(offsetof(OrbisVrTrackerResultData, led) + offsetof(OrbisVrTrackerLedResult, rx) ==
              0x1f8);
static_assert(sizeof(OrbisVrTrackerLedResult) * ORBIS_VR_TRACKER_MAX_LED_NUM == 0x200);

inline void WriteVirtualCameraMarker(OrbisVrTrackerResultData& result,
                                     const VirtualCameraMarker& marker, u64 timestamp) {
    result.timestamp_of_led_result = timestamp;
    for (unsigned camera = 0; camera < Libraries::Camera::ORBIS_CAMERA_MAX_DEVICE_NUM; ++camera) {
        result.number_of_led_result[camera] = 1;
        auto& led = result.led[camera][0];
        led.x = marker.x;
        led.y = marker.y;
        // Blood & Truth consumes this pair as normalized image coordinates.
        led.rx = marker.x;
        led.ry = marker.y;
    }
}
} // namespace Libraries::VrTracker
