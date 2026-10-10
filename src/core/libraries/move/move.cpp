// SPDX-FileCopyrightText: Copyright 2024-2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#include <array>
#include <chrono>
#include <mutex>
#include "common/logging/log.h"
#include "core/libraries/error_codes.h"
#include "core/libraries/libs.h"
#include "core/libraries/move/move.h"
#include "core/libraries/move/move_error.h"
#include "core/vr/vr_runtime.h"

namespace Libraries::Move {
static std::mutex move_mutex;
static bool initialized{};
static std::array<s32, 2> handles{-1, -1};
static s32 next_handle = 0x30b0000;

static int FindHand(s32 handle) {
    for (int hand = 0; hand < 2; ++hand)
        if (handles[hand] == handle && handle >= 0)
            return hand;
    return -1;
}
int HandForHandle(s32 handle) {
    std::scoped_lock lock{move_mutex};
    return initialized ? FindHand(handle) : -1;
}
bool HasOpenControllerPair() {
    std::scoped_lock lock{move_mutex};
    return initialized && handles[0] >= 0 && handles[1] >= 0;
}
s32 PS4_SYSV_ABI sceMoveInit() {
    std::scoped_lock lock{move_mutex};
    if (initialized)
        return ORBIS_MOVE_ERROR_ALREADY_INIT;
    initialized = true;
    handles = {-1, -1};
    LOG_INFO(Lib_Move, "Two OpenXR Move controllers enabled: index 0 left, index 1 right");
    return ORBIS_OK;
}
s32 PS4_SYSV_ABI sceMoveOpen(Libraries::UserService::OrbisUserServiceUserId user_id, s32 type,
                             s32 index) {
    std::scoped_lock lock{move_mutex};
    if (!initialized)
        return ORBIS_MOVE_ERROR_NOT_INIT;
    if (index < 0 || index >= 2)
        return ORBIS_MOVE_ERROR_INVALID_PORT;
    if (handles[index] >= 0)
        return ORBIS_MOVE_ERROR_ALREADY_OPENED;
    handles[index] = (next_handle += 0x100);
    LOG_INFO(Lib_Move, "Opened Move index {} handle {:#x} user {} type {}", index, handles[index],
             user_id, type);
    return handles[index];
}
s32 PS4_SYSV_ABI sceMoveGetDeviceInfo(s32 handle, OrbisMoveDeviceInfo* info) {
    std::scoped_lock lock{move_mutex};
    if (!initialized)
        return ORBIS_MOVE_ERROR_NOT_INIT;
    if (!info)
        return ORBIS_MOVE_ERROR_INVALID_ARG;
    const auto hand = FindHand(handle);
    if (hand < 0)
        return ORBIS_MOVE_ERROR_INVALID_HANDLE;
    *info = {};
    if (!Core::Vr::Runtime::Instance().GetMove(hand).connected)
        return ORBIS_MOVE_ERROR_NO_CONTROLLER_CONNECTED;
    info->sphere_radius = 0.0225f;
    return ORBIS_OK;
}
static s32 Read(s32 handle, OrbisMoveData* data) {
    if (!initialized)
        return ORBIS_MOVE_ERROR_NOT_INIT;
    if (!data)
        return ORBIS_MOVE_ERROR_INVALID_ARG;
    const auto hand = FindHand(handle);
    if (hand < 0)
        return ORBIS_MOVE_ERROR_INVALID_HANDLE;
    *data = {};
    const auto state = Core::Vr::Runtime::Instance().GetMove(hand);
    // Observe the guest-facing samples, including disconnection, without logging every poll.
    static std::array<std::chrono::steady_clock::time_point, 2> last_report{};
    const auto now = std::chrono::steady_clock::now();
    if (now - last_report[hand] >= std::chrono::seconds{10}) {
        last_report[hand] = now;
        LOG_INFO(Lib_Move,
                 "Move sample hand {}: connected {}, tracked {}, buttons {:#x}, trigger {}, "
                 "timestamp {}, sequence {}, position ({:.3f}, {:.3f}, {:.3f})",
                 hand, state.connected, state.device.tracked, state.buttons, state.trigger,
                 state.timestamp, state.device.sequence, state.device.pose.position.x,
                 state.device.pose.position.y, state.device.pose.position.z);
    }
    if (!state.connected)
        return ORBIS_MOVE_ERROR_NO_CONTROLLER_CONNECTED;
    data->button_data = {state.buttons, state.trigger};
    data->timestamp = state.timestamp;
    data->count = static_cast<s32>(state.device.sequence & 0x7fffffff);
    data->temperature = 25.0f;
    // Optical poses come from VrTracker; sensor vectors are device-local.
    const auto inverse = Core::Vr::Conjugate(state.device.pose.orientation);
    const auto gravity = Core::Vr::Rotate(inverse, {0.0f, 1.0f, 0.0f});
    const auto angular = Core::Vr::Rotate(inverse, state.device.angular_velocity);
    data->accelerometer[0] = gravity.x;
    data->accelerometer[1] = gravity.y;
    data->accelerometer[2] = gravity.z;
    data->gyro[0] = angular.x;
    data->gyro[1] = angular.y;
    data->gyro[2] = angular.z;
    return ORBIS_OK;
}
s32 PS4_SYSV_ABI sceMoveReadStateLatest(s32 handle, OrbisMoveData* data) {
    std::scoped_lock lock{move_mutex};
    return Read(handle, data);
}
s32 PS4_SYSV_ABI sceMoveReadStateRecent(s32 handle, s64 timestamp, OrbisMoveData* data,
                                        s32* out_count) {
    std::scoped_lock lock{move_mutex};
    if (!initialized)
        return ORBIS_MOVE_ERROR_NOT_INIT;
    if (timestamp < 0 || !data || !out_count)
        return ORBIS_MOVE_ERROR_INVALID_ARG;
    *out_count = 0;
    OrbisMoveData latest{};
    const auto result = Read(handle, &latest);
    if (result != ORBIS_OK)
        return result;
    if (latest.timestamp > timestamp) {
        *data = latest;
        *out_count = 1;
    }
    return ORBIS_OK;
}
s32 PS4_SYSV_ABI sceMoveGetExtensionPortInfo(s32 handle, void* data) {
    std::scoped_lock lock{move_mutex};
    if (!initialized)
        return ORBIS_MOVE_ERROR_NOT_INIT;
    if (!data)
        return ORBIS_MOVE_ERROR_INVALID_ARG;
    if (FindHand(handle) < 0)
        return ORBIS_MOVE_ERROR_INVALID_HANDLE;
    return ORBIS_MOVE_ERROR_NO_CONTROLLER_CONNECTED;
}
s32 PS4_SYSV_ABI sceMoveSetVibration(s32 handle, u8 intensity) {
    std::scoped_lock lock{move_mutex};
    if (!initialized)
        return ORBIS_MOVE_ERROR_NOT_INIT;
    const auto hand = FindHand(handle);
    if (hand < 0)
        return ORBIS_MOVE_ERROR_INVALID_HANDLE;
    Core::Vr::Runtime::Instance().SetMoveVibration(hand, intensity);
    return Core::Vr::Runtime::Instance().GetMove(hand).connected
               ? ORBIS_OK
               : ORBIS_MOVE_ERROR_NO_CONTROLLER_CONNECTED;
}
s32 PS4_SYSV_ABI sceMoveSetLightSphere(s32 handle, u8 red, u8 green, u8 blue) {
    std::scoped_lock lock{move_mutex};
    if (!initialized)
        return ORBIS_MOVE_ERROR_NOT_INIT;
    return FindHand(handle) < 0 ? ORBIS_MOVE_ERROR_INVALID_HANDLE : ORBIS_OK;
}
s32 PS4_SYSV_ABI sceMoveResetLightSphere(s32 handle) {
    return sceMoveSetLightSphere(handle, 0, 0, 255);
}
s32 PS4_SYSV_ABI sceMoveClose(s32 handle) {
    std::scoped_lock lock{move_mutex};
    if (!initialized)
        return ORBIS_MOVE_ERROR_NOT_INIT;
    const auto hand = FindHand(handle);
    if (hand < 0)
        return ORBIS_MOVE_ERROR_INVALID_HANDLE;
    Core::Vr::Runtime::Instance().SetMoveVibration(hand, 0);
    handles[hand] = -1;
    return ORBIS_OK;
}
s32 PS4_SYSV_ABI sceMoveTerm() {
    std::scoped_lock lock{move_mutex};
    if (!initialized)
        return ORBIS_MOVE_ERROR_NOT_INIT;
    handles = {-1, -1};
    initialized = false;
    Core::Vr::Runtime::Instance().ReleaseMoves();
    return ORBIS_OK;
}

void RegisterLib(Core::Loader::SymbolsResolver* sym) {
    LIB_FUNCTION("j1ITE-EoJmE", "libSceMove", 1, "libSceMove", sceMoveInit);
    LIB_FUNCTION("HzC60MfjJxU", "libSceMove", 1, "libSceMove", sceMoveOpen);
    LIB_FUNCTION("GWXTyxs4QbE", "libSceMove", 1, "libSceMove", sceMoveGetDeviceInfo);
    LIB_FUNCTION("ttU+JOhShl4", "libSceMove", 1, "libSceMove", sceMoveReadStateLatest);
    LIB_FUNCTION("f2bcpK6kJfg", "libSceMove", 1, "libSceMove", sceMoveReadStateRecent);
    LIB_FUNCTION("y5h7f8H1Jnk", "libSceMove", 1, "libSceMove", sceMoveGetExtensionPortInfo);
    LIB_FUNCTION("IFQwtT2CeY0", "libSceMove", 1, "libSceMove", sceMoveSetVibration);
    LIB_FUNCTION("T8KYHPs1JE8", "libSceMove", 1, "libSceMove", sceMoveSetLightSphere);
    LIB_FUNCTION("zuxWAg3HAac", "libSceMove", 1, "libSceMove", sceMoveResetLightSphere);
    LIB_FUNCTION("XX6wlxpHyeo", "libSceMove", 1, "libSceMove", sceMoveClose);
    LIB_FUNCTION("tsZi60H4ypY", "libSceMove", 1, "libSceMove", sceMoveTerm);
};

} // namespace Libraries::Move
