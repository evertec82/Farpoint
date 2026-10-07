# Farpoint PC VR development build

This branch adapts AstroQuest's shadPS4/OpenXR host to **Farpoint CUSA04508, version 1.00**. It is a prototype: startup and stereo calibration rendering are verified; campaign gameplay, weapon alignment, audio, and saves across a complete play session are not yet verified.

## Start the installed build

1. Connect the headset and controllers through your active OpenXR runtime. SteamVR was used for the startup test.
2. Open `Play Farpoint.bat` in the installation folder (`D:\farpoint` on the development machine).
3. Choose **VR controllers / tracked Aim weapon** or **Gamepad / DualShock mode** and start. The verified output layout is **Original PSVR - 960 x 1080 per eye**.
4. Complete the game's camera-height/controller calibration. Use the on-screen hold-to-skip instruction if needed. In VR-controller mode right A maps to Cross; right B maps to Square; left X/Y map to Circle/Triangle. Left stick moves, right stick turns, triggers map to L2/R2, grips to L1/R1, and left menu to Options. Both stick clicks recenter the host view. The right controller supplies the experimental Aim pose.

The launcher checks the executable version, keeps a separate Farpoint user profile, disables online services, and retains the game's native stereo output layout and timing. Desktop mirror options are 60 FPS, 30 FPS, and uncapped. They do not select a headset refresh rate. AstroBot's resolution, physics, and FPS-unlock patches are not applied to Farpoint.

## What changed

- Reuse missing-import stub slots across modules so repeated relocations do not exhaust the named-stub table.
- Register the existing C++ static-initialization guards on native Windows, preventing Farpoint's NP toolkit from using unconstructed singleton objects.
- Supply guest-format C-locale classification and case-conversion tables needed by the toolkit.
- Permit Farpoint to create a local offline score context, avoiding its unconditional leaderboard initialization fatal error. This does not provide PSN or multiplayer.
- Bridge Farpoint's observed single-color-layer PSVR submission to the existing OpenXR pipeline. Split its packed stereo texture and recover each eye's projection instead of showing the full stereo texture in both eyes. Clamp shader sampling to the selected eye to avoid bleeding across the stereo seam.
- Expose tracked Aim poses and a conventional VR-controller gamepad mapping without AstroBot's touchpad gestures.
- Retain the inherited queue/timeline synchronization, shader-cache repair, precise guest timers, desktop mirror limiting, late-frame handling, and SteamVR compatibility work.

## Validation and limitations

On October 7, 2026, the supplied package was extracted locally and three successive startup failures were diagnosed and fixed. The game remained alive for 55 seconds and rendered its stereo camera-height calibration screen. It submitted about 45 game frames per second while SteamVR reported a 90 Hz runtime. **This was an idle, synchronized-only headset session, not a worn-headset gameplay benchmark.** The inactive runtime throttled actual presentation; these numbers do not establish headset smoothness.

`tools/tests/farpoint_compat_test.cpp` exercises concurrent static construction, aborted construction/retry, the guest character tables, and packed-eye projection/sampling coordinates. The release build and shader compilation pass.

Only one color layer has been observed and implemented. Additional layers, depth-based reprojection, the remaining multilayer common-data ABI, and all campaign interactions need further work if encountered. No claim of a fully playable port is made. A disconnected VR controller is reported as untracked rather than assigned a synthetic weapon position.

Logs: `pc-vr/user/log/shad_log.txt`. Local saves and caches are under `pc-vr/user`. The original package and AstroBot installation are preserved. Game files, extracted executables/modules, logs, render-target dumps, and save data must stay outside source control and public release archives.

## Source and build

The branch starts at AstroQuest commit `3a75299ff5216f3f255a987d462cc974fcba9eb3`. Its original build instructions and license remain in this repository. With initialized upstream submodules, a Windows clang-cl toolchain and SDK, configure `shadps4-arm64-main` with Ninja in Release mode, `ENABLE_OPENXR=ON`, `ENABLE_UPDATER=OFF`, `ENABLE_DISCORD_RPC=OFF`, and `ALSOFT_UPDATE_BUILD_VERSION=OFF`. `tools/build-farpoint.ps1` supports the portable toolchain layout used locally. Run `tools/build-farpoint.ps1 -Test` to execute the compatibility checks. Build output is excluded from Git.

The supplied executable SHA-256 is `92A21FF9E309CE5DD58B058C4B12329463697BB27FDA98B5DA353E0A73BD9C0F`. Other regions or updates need separate ABI/startup verification before this launcher will accept them.

## Calibration/input follow-up

The initial build rejected Farpoint's 144-byte tracker-init ABI; its virtual camera and head tracking never started. The updated build translates the observed fields into the existing 128-byte HLE parameters and retains memory/parameter validation. Camera frames and tracker result requests now run. Farpoint also opens standard and Aim pad ports together: both now receive the player's input in Aim mode, while the standard port retains its standard device class. Play-area warning output is initialized instead of left unwritten. Regression tests cover the tracker memory layout and pad-port routing. Live headset interaction still needs retesting. One scripted diagnostic run encountered a separate Unreal asset serialization failure while loading SpikeGunBP; later startup tests survived 45 seconds, but campaign stability is not established.

## Resolution and lighting follow-up

The experimental larger output buffers exposed a stereo regression: Farpoint still placed its eye views in the original-size portion of the texture. The patch and Sharp launcher option have been removed. Submitted texture dimensions alone do not establish correct higher-resolution rendering.

A targeted texture-cache change preserves Farpoint's 32-cubed color-grading LUT when the game switches between array and volume views. Startup tests exercise this path, but it did not resolve the reported lighting issue on its own.

The current local lighting workaround uses a configuration-only override pak selecting the 16-bit floating-point HDR scene format (`r.SceneColorFormat=4`) and disabling bloom (`r.BloomQuality=0`). Original game paks remain unchanged. The override contains extracted configuration data and must stay out of Git and public archives. This is a diagnostic workaround awaiting a headset check, not an established emulator rendering fix. Resolution and other game settings are retained.
## Campaign-loading crash follow-up

The Start Game crash was reproduced as an Unreal out-of-memory fatal error with implausible, varying multi-gigabyte allocation requests. All 10,265 payload hashes in the main game pak matched their stored checksums. A read trace then caught a zero-byte read well before EOF; a second run recorded Windows `ERROR_INVALID_USER_BUFFER` (1784). Windows cannot perform an OS file read into guest pages protected by GPU memory tracking through the normal CPU fault handler.

Windows kernel file reads now use a bounded, reusable 1 MiB host staging buffer and copy into guest memory with CPU stores, allowing tracking faults to be handled. The protected-page regression test covers data spanning multiple chunks, a short final read, untouched trailing bytes, EOF, and a zero-length request. Scripted calibration/Start Game testing survived successive 65- and 100-second runs after this change; equivalent earlier runs crashed while loading. This is not a complete campaign stability claim. Disabling the separate async-loading thread did not solve the issue and is not retained.
