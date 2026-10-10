// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <chrono>
#include <condition_variable>
#include <mutex>
#include "common/types.h"
namespace Core::Vr::GuestFrames {
inline std::mutex mutex;
inline std::condition_variable changed;
inline u64 count{};
inline void NoteGuestProgress() noexcept {
    { std::lock_guard lock(mutex); ++count; }
    changed.notify_all();
}
inline u64 GuestFrameCount() noexcept {
    std::lock_guard lock(mutex); return count;
}
inline u64 WaitForGuestFrame(u64 target, std::chrono::milliseconds timeout) {
    std::unique_lock lock(mutex);
    changed.wait_for(lock, timeout, [&] { return count >= target; });
    return count;
}
}
