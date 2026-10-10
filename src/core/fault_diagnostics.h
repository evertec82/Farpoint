// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "common/arch.h"
#include "common/logging/log.h"
#if defined(_WIN32) && defined(ARCH_X86_64)
#include <windows.h>
namespace Core {
inline void ReportFaultMemory(const CONTEXT& context) {
    LOG_CRITICAL(Debug, "Crash diagnostic build: farpoint-pointer-watch-20261009");
    const unsigned long long addresses[]{context.Rax, context.Rsi, context.Rdi, context.Rsp,
                                        context.Rbp};
    for (const auto address : addresses) {
        MEMORY_BASIC_INFORMATION info{};
        if (VirtualQuery(reinterpret_cast<void*>(address), &info, sizeof(info))) {
            LOG_CRITICAL(Debug,
                         "Fault mapping address={:#x} base={} allocation={} size={:#x} "
                         "state={:#x} protection={:#x} type={:#x}",
                         address, info.BaseAddress, info.AllocationBase, info.RegionSize,
                         info.State, info.Protect, info.Type);
        }
    }
    unsigned long long code[4]{};
    SIZE_T copied{};
    if (ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(context.Rip), code,
                          sizeof(code), &copied) && copied == sizeof(code)) {
        LOG_CRITICAL(Debug, "Fault instruction words (little endian): {:#018x} {:#018x} {:#018x} {:#018x}",
                     code[0], code[1], code[2], code[3]);
    }
    auto frame = context.Rbp;
    for (unsigned i = 0; i < 12 && frame >= context.Rsp && frame - context.Rsp < 0x20000; ++i) {
        unsigned long long words[2]{};
        if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(frame), words,
                               sizeof(words), &copied) || copied != sizeof(words)) {
            break;
        }
        LOG_CRITICAL(Debug, "Guest frame {} rbp={:#x} return={:#x}", i, frame, words[1]);
        if (words[0] <= frame) {
            break;
        }
        frame = words[0];
    }
}
}
#endif
