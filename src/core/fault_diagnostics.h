// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "common/arch.h"
#include "common/logging/log.h"
#include "core/allocation_diagnostics.h"
#if defined(_WIN32) && defined(ARCH_X86_64)
#include <windows.h>
#include <cstdio>
namespace Core {
inline void SaveFaultContext(const CONTEXT& c) {
    char path[MAX_PATH]{};
    std::snprintf(path, sizeof(path), "user/log/fault-context-%lu-%lu-%llu.txt",
                  GetCurrentProcessId(), GetCurrentThreadId(), GetTickCount64());
    const HANDLE file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                                    CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                                    nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }
    const auto write = [file](const char* text, int length) {
        if (length > 0) {
            DWORD written{};
            WriteFile(file, text, static_cast<DWORD>(length), &written, nullptr);
        }
    };
    char line[512]{};
    const unsigned long long registers[]{c.Rax, c.Rbx, c.Rcx, c.Rdx, c.Rsi, c.Rdi,
                                        c.Rsp, c.Rbp, c.R8, c.R9, c.R10, c.R11,
                                        c.R12, c.R13, c.R14, c.R15, c.Rip};
    const char* names[]{"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rsp", "rbp",
                        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "rip"};
    for (unsigned i = 0; i < 17; ++i) {
        write(line, std::snprintf(line, sizeof(line), "%s=0x%llx\n", names[i], registers[i]));
    }
    const auto snapshot = [&](unsigned long long address) {
        unsigned long long words[32]{};
        SIZE_T copied{};
        ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(address), words,
                          sizeof(words), &copied);
        write(line, std::snprintf(line, sizeof(line), "memory 0x%llx bytes=%llu\n",
                                  address, static_cast<unsigned long long>(copied)));
        for (SIZE_T i = 0; i < copied / sizeof(words[0]); ++i) {
            write(line, std::snprintf(line, sizeof(line), "0x%llx: 0x%016llx\n",
                                      address + i * sizeof(words[0]), words[i]));
        }
    };
    if (c.Rsp >= 256) {
        snapshot(c.Rsp - 256);
    }
    snapshot(c.Rsp);
    if (c.Rbp >= 256) {
        snapshot(c.Rbp - 256);
    }
    snapshot(c.Rbp);
    for (const auto address : registers) {
        snapshot(address);
    }
    FlushFileBuffers(file);
    CloseHandle(file);
    SaveAllocationHistory();
}

inline void ReportFaultWords(const char* kind, unsigned long long address) {
    unsigned long long words[32]{};
    SIZE_T copied{};
    ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(address), words,
                      sizeof(words), &copied);
    for (SIZE_T i = 0; i < copied / sizeof(words[0]); i += 4) {
        LOG_CRITICAL(Debug, "Fault snapshot {} address={:#x} words={:#018x} {:#018x} {:#018x} {:#018x}",
                     kind, address + i * sizeof(words[0]), words[i], words[i + 1],
                     words[i + 2], words[i + 3]);
    }
}

inline void ReportFaultMemory(const CONTEXT& context) {
    LOG_CRITICAL(Debug, "Crash diagnostic build: fault-object-snapshots-20261010");
    const unsigned long long addresses[]{context.Rax, context.Rbx, context.Rcx, context.Rdx,
                                        context.Rsi, context.Rdi, context.Rsp, context.Rbp,
                                        context.R8, context.R9, context.R10, context.R11,
                                        context.R12, context.R13, context.R14, context.R15};
    for (const auto address : addresses) {
        ReportFaultWords("register", address);
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
        if (frame >= 256) {
            ReportFaultWords("frame-locals", frame - 256);
        }
        ReportFaultWords("frame-arguments", frame);
        if (words[0] <= frame) {
            break;
        }
        frame = words[0];
    }
}
}
#endif
