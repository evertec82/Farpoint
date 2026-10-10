// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <chrono>
#include <thread>
#ifdef _WIN32
#include "common/windows_sleep_timer.h"
#endif
namespace Core::Vr {
class PresentationTimer {
    using Clock = std::chrono::steady_clock;
    std::chrono::nanoseconds interval;
    Clock::time_point next{Clock::now()};
#ifdef _WIN32
    Common::WindowsSleepTimer timer;
#endif
public:
    explicit PresentationTimer(std::chrono::nanoseconds interval, bool precise) : interval(interval)
#ifdef _WIN32
      , timer(precise)
#endif
    {}
    void SetInterval(std::chrono::nanoseconds value) { interval = value; }
    void Start() {
        const auto now = Clock::now();
        if (next > now) {
#ifdef _WIN32
            if (timer.Wait(next - now, false) == WAIT_FAILED) std::this_thread::sleep_until(next);
#else
            std::this_thread::sleep_until(next);
#endif
        } else if (now - next > interval) { next = now; }
    }
    void End() { next += interval; }
    std::chrono::nanoseconds GetTotalWait() const { return next - Clock::now(); }
};
}

