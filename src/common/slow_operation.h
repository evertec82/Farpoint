// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <chrono>
#include <cstdlib>
#include <string_view>
#include <fmt/format.h>
#include "common/logging/log.h"

namespace Common {
class SlowOperation {
public:
    explicit SlowOperation(std::string_view name_) : name{name_} {
        static const bool enabled = [] {
            const char* value = std::getenv("SHADPS4_GPU_WAIT_TIMING");
            return value && value[0] == '1';
        }();
        active = enabled;
        if (active) {
            start = Clock::now();
        }
    }

    ~SlowOperation() {
        if (!active) {
            return;
        }
        const auto end = Clock::now();
        const double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
        if (elapsed >= 20.0) {
            LOG_INFO(Core, "Slow operation {}: {:.2f} ms, ending at steady_ms={}", name,
                     elapsed, std::chrono::duration_cast<std::chrono::milliseconds>(
                                  end.time_since_epoch()).count());
        }
    }

private:
    using Clock = std::chrono::steady_clock;
    std::string_view name;
    Clock::time_point start{};
    bool active{};
};
} // namespace Common
