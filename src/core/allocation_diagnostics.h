// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cstdio>
#include "common/types.h"
#ifdef _WIN32
#include <windows.h>
#endif

namespace Core {
struct AllocationEvent {
    u64 sequence{}, time{}, thread{}, address{}, size{}, physical{}, caller{};
    s64 result{};
    const char* operation{};
};
struct AllocationSlot {
    std::atomic_flag busy = ATOMIC_FLAG_INIT;
    AllocationEvent event;
};
inline std::array<AllocationSlot, 4096> allocation_history;
inline std::atomic<u64> allocation_sequence{};
inline std::array<char, 4096 * 384> allocation_dump_buffer;
inline std::atomic_flag allocation_dump_busy = ATOMIC_FLAG_INIT;

inline void TraceAllocation(const char* operation, u64 address, u64 size, u64 physical,
                            s64 result, const void* caller) {
#ifdef _WIN32
    static const bool enabled = std::getenv("SHADPS4_ALLOCATION_TRACE") != nullptr;
    if (!enabled) {
        return;
    }
    const auto sequence = allocation_sequence.fetch_add(1) + 1;
    auto& slot = allocation_history[sequence % allocation_history.size()];
    if (slot.busy.test_and_set(std::memory_order_acquire)) {
        return;
    }
    slot.event = {sequence, GetTickCount64(), GetCurrentThreadId(), address, size,
                  physical, reinterpret_cast<u64>(caller), result, operation};
    slot.busy.clear(std::memory_order_release);
#endif
}

inline void SaveAllocationHistory() {
#ifdef _WIN32
    const auto end = allocation_sequence.load();
    if (!end) {
        return;
    }
    if (allocation_dump_busy.test_and_set(std::memory_order_acquire)) {
        return;
    }
    char path[MAX_PATH]{};
    std::snprintf(path, sizeof(path), "user/log/allocation-history-%lu-%llu.txt",
                  GetCurrentProcessId(), GetTickCount64());
    const auto file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW,
                                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        allocation_dump_busy.clear(std::memory_order_release);
        return;
    }
    size_t used = 0;
    const u64 begin = end > allocation_history.size() ? end - allocation_history.size() + 1 : 1;
    for (u64 i = begin; i <= end; ++i) {
        auto& slot = allocation_history[i % allocation_history.size()];
        if (slot.busy.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        const auto event = slot.event;
        slot.busy.clear(std::memory_order_release);
        if (event.sequence != i) {
            continue;
        }
        char line[384]{};
        const auto length = std::snprintf(line, sizeof(line),
            "%llu time=%llu thread=%llu %s address=%#llx size=%#llx physical=%#llx result=%lld caller=%#llx\n",
            event.sequence, event.time, event.thread, event.operation, event.address,
            event.size, event.physical, event.result, event.caller);
        if (length > 0 && length < sizeof(line)) {
            std::copy_n(line, length, allocation_dump_buffer.data() + used);
            used += length;
        }
    }
    DWORD written{};
    WriteFile(file, allocation_dump_buffer.data(), static_cast<DWORD>(used), &written, nullptr);
    FlushFileBuffers(file);
    CloseHandle(file);
    allocation_dump_busy.clear(std::memory_order_release);
#endif
}
}
