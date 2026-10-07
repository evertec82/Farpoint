// SPDX-License-Identifier: GPL-2.0-or-later
#include "core/libraries/libc_internal/guest_ctype.h"
#include "core/libraries/libc_internal/libc_internal_cxa.h"
#include "core/libraries/vr_tracker/tracker_compat.h"
#include "core/vr/stereo_layout.h"
#include "input/pad_routing.h"
#include <atomic>
#include <cassert>
#include <cmath>
#include <thread>
#include <vector>
int main() {
  using namespace Libraries::VrTracker;
  std::array<unsigned char, 144> wire{};
  auto put32 = [&](std::size_t offset, u32 value) {
    std::memcpy(wire.data() + offset, &value, 4);
  };
  auto put64 = [&](std::size_t offset, u64 value) {
    std::memcpy(wire.data() + offset, &value, 8);
  };
  put32(0, 144);
  put32(0x20, 0);
  put32(0x24, 1);
  put32(0x28, 1);
  put32(0x2c, 1);
  put64(0x40, 0x29ec40000);
  put32(0x48, 0x800000);
  put32(0x4c, 0x10000);
  put64(0x50, 0x20f280000);
  put32(0x58, 0x3000000);
  put32(0x5c, 0x10000);
  put64(0x60, 0x200050000);
  put32(0x68, 0x1000000);
  put32(0x6c, 0x10000);
  put32(0x70, 6);
  OrbisVrTrackerInitParam decoded{};
  assert(DecodeFarpointTrackerInit144(wire.data(), wire.size(), decoded));
  assert(decoded.size == 128 && decoded.cpu_mask == 0);
  assert(decoded.calibration_settings.hmd_position == 0 &&
         decoded.calibration_settings.move_position == 1);
  assert(reinterpret_cast<u64>(decoded.direct_memory_onion) == 0x29ec40000);
  assert(decoded.direct_memory_onion_size == 0x800000 &&
         decoded.direct_memory_garlic_size == 0x3000000);
  assert(reinterpret_cast<u64>(decoded.work_memory) == 0x200050000 &&
         decoded.gpu_pipe_id == 6);
  assert(!DecodeFarpointTrackerInit144(wire.data(), 128, decoded));

  assert(Input::PlayerUsesPadPort(true, true, 0));
  assert(Input::PlayerUsesPadPort(true, true, 2));
  assert(!Input::PlayerUsesPadPort(false, true, 0));
  assert(!Input::PlayerUsesPadPort(true, false, 2));
  assert(Input::PlayerUsesPadPort(true, false, 0));
  assert(!Input::PadPortIsSpecial(true, true, 0));
  assert(Input::PadPortIsSpecial(true, true, 2));
  using namespace Libraries::LibcInternal;
  const auto *classes = GuestCtype.classes.data() + 128;
  const auto *lower = GuestCtype.lower.data() + 128;
  const auto *upper = GuestCtype.upper.data() + 128;
  assert(classes[-1] == 0 && lower[-1] == -1 && upper[-1] == -1);
  for (int c = 0; c < 256; ++c) {
    assert(bool(classes[c] & 0x02) == (c >= 'A' && c <= 'Z'));
    assert(bool(classes[c] & 0x10) == (c >= 'a' && c <= 'z'));
    assert(bool(classes[c] & 0x20) == (c >= '0' && c <= '9'));
    assert(bool(classes[c] & 0x01) ==
           ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
            (c >= 'a' && c <= 'f')));
    assert(lower[c] == (c >= 'A' && c <= 'Z' ? c + 32 : c));
    assert(upper[c] == (c >= 'a' && c <= 'z' ? c - 32 : c));
  }
  // The initializer must run exactly once and its writes must be published to
  // waiters.
  u64 guard = 0;
  int constructed = 0;
  std::atomic<int> initializers = 0;
  std::vector<std::thread> workers;
  for (int i = 0; i < 16; ++i)
    workers.emplace_back([&] {
      if (fex_libc_cxa_guard_acquire(&guard)) {
        ++initializers;
        constructed = 12345;
        fex_libc_cxa_guard_release(&guard);
      }
      assert(constructed == 12345);
    });
  for (auto &worker : workers)
    worker.join();
  assert(initializers == 1);
  u64 retry = 0;
  assert(fex_libc_cxa_guard_acquire(&retry) == 1);
  fex_libc_cxa_guard_abort(&retry);
  assert(fex_libc_cxa_guard_acquire(&retry) == 1);
  fex_libc_cxa_guard_release(&retry);
  assert(fex_libc_cxa_guard_acquire(&retry) == 0);
  // A stereo texture must sample different eye centers, without distorting
  // projection.
  const auto left = Core::Vr::EyeSourceUv(true, 0);
  const auto right = Core::Vr::EyeSourceUv(true, 1);
  assert(0.5f * left[0] + left[2] == 0.25f);
  assert(0.5f * right[0] + right[2] == 0.75f);
  assert(Core::Vr::EyeSourceUv(false, 1)[0] == 1.0f);
  const auto unpacked =
      Core::Vr::LeftEyeProjectionUv({0.2f, 0.4f, 0.26f, 0.5f}, true);
  assert(std::abs(unpacked[2] / unpacked[0] - 1.3f) < 0.0001f);
  assert(std::abs((1.0f - unpacked[2]) / unpacked[0] - 1.2f) < 0.0001f);
}
