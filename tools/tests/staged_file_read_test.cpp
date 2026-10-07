// SPDX-License-Identifier: GPL-2.0-or-later
#define NOMINMAX
#include "common/staged_file_read.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <windows.h>
static unsigned char *tracked;
static std::size_t tracked_size;
static unsigned faults;
static LONG CALLBACK Untrack(EXCEPTION_POINTERS *exception) {
  if (exception->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION ||
      exception->ExceptionRecord->ExceptionInformation[0] != 1) {
    return EXCEPTION_CONTINUE_SEARCH;
  }
  const auto address = exception->ExceptionRecord->ExceptionInformation[1];
  if (address < reinterpret_cast<ULONG_PTR>(tracked) ||
      address >= reinterpret_cast<ULONG_PTR>(tracked) + tracked_size) {
    return EXCEPTION_CONTINUE_SEARCH;
  }
  DWORD old;
  if (!VirtualProtect(tracked, tracked_size, PAGE_READWRITE, &old)) {
    return EXCEPTION_CONTINUE_SEARCH;
  }
  ++faults;
  return EXCEPTION_CONTINUE_EXECUTION;
}
int main() {
  // More than one staging chunk, plus a short final read and untouched tail.
  constexpr std::size_t count = 1500000;
  tracked_size = 2 * 1024 * 1024;
  tracked = static_cast<unsigned char *>(VirtualAlloc(
      nullptr, tracked_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
  assert(tracked);
  std::memset(tracked, 0xcc, tracked_size);
  std::vector<unsigned char> source(count);
  for (std::size_t i = 0; i < count; ++i) {
    source[i] = (i * 37 + i / 251) & 255;
  }
  FILE *file = nullptr;
  assert(tmpfile_s(&file) == 0);
  assert(file);
  assert(std::fwrite(source.data(), 1, source.size(), file) == count);
  std::rewind(file);
  auto *handler = AddVectoredExceptionHandler(1, Untrack);
  assert(handler);
  DWORD old;
  assert(VirtualProtect(tracked, tracked_size, PAGE_READONLY, &old));
  const auto read = Common::FS::ReadStaged(
      tracked, tracked_size, [&](void *buffer, std::size_t size) {
        return std::fread(buffer, 1, size, file);
      });
  assert(read == count);
  assert(faults != 0);
  assert(std::memcmp(tracked, source.data(), count) == 0);
  for (auto i = count; i < tracked_size; ++i) {
    assert(tracked[i] == 0xcc);
  }
  assert(std::ftell(file) == count);
  assert(
      Common::FS::ReadStaged(tracked, 100, [&](void *buffer, std::size_t size) {
        return std::fread(buffer, 1, size, file);
      }) == 0);
  bool invoked = false;
  assert(Common::FS::ReadStaged(tracked, 0, [&](void *, std::size_t) {
           invoked = true;
           return std::size_t{0};
         }) == 0);
  assert(!invoked);
  std::fclose(file);
  RemoveVectoredExceptionHandler(handler);
  VirtualFree(tracked, 0, MEM_RELEASE);
}
