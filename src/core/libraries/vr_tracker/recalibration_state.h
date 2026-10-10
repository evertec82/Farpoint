// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstdint>
namespace Libraries::VrTracker {
// Each independently tracked device must observe the calibration transition.
struct RecalibrationState {
    std::uint64_t until{};
    std::uint32_t reported = ~std::uint32_t{0};
    void Begin(std::uint64_t deadline) { until = deadline; reported = 0; }
    bool IsActive(std::uint64_t now, unsigned device) {
        if (device >= 32) return false;
        const auto bit = std::uint32_t{1} << device;
        if ((reported & bit) && now >= until) return false;
        reported |= bit;
        return true;
    }
};
}
