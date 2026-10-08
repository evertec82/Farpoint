# Farpoint PC VR development build

Farpoint CUSA04508 version 1.00 running through AstroQuest's shadPS4/OpenXR host on Windows. This is a development port, with known rendering defects and incomplete campaign validation.

## Install this update

The release ZIP is an **update for an existing Farpoint PC VR installation**, not a complete game installer. Close the emulator, back up your installation, then extract the ZIP into its root so `pc-vr/shadps4.exe` and `pc-vr/launch-farpoint.ps1` replace the existing files. Open `Play Farpoint.bat`.

You must supply your own extracted game at `games/CUSA04508/files/uroot/eboot.bin`, existing `pc-vr/user/config.json`, and locally generated resolution profiles in `pc-vr/resolution-profiles/`. These packs contain configuration extracted from the game and are deliberately not distributed. An installation lacking them cannot launch with this update. Saves and emulator settings are not included or overwritten by the archive.

Supported game executable SHA-256: `92A21FF9E309CE5DD58B058C4B12329463697BB27FDA98B5DA353E0A73BD9C0F`.

## Current changes

- Correct packed stereo submission and per-eye sampling for OpenXR, including SteamVR.
- Fix tracker initialization and controller routing; tracked Aim and gamepad modes are available.
- Stage Windows file reads before copying into protected guest memory, addressing a campaign-loading crash.
- Correct small reflection mip matching and texture-array subresource offsets, addressing green lighting and materials.
- Complete the game's sky-brightness readback before signaling GPU completion, addressing excessively bright terrain. Uses original game shaders.
- Enable full-refresh Farpoint pacing instead of the default half-refresh wait. The compositor still handles its own reprojection; fresh frames at headset refresh are not guaranteed in every scene.
- Offer 960x1080, 1536x1728, 1920x2160, 2160x2430, 2400x2700, 2688x3024 and 3072x3456 per eye. The maximum is 10.24 times the original pixel count. Scaling uses the game's stereo renderer rather than changing panel dimensions.
- Reserve additional guest direct memory for supersampling: at least 2 GiB extra at 1536/1920, 4 GiB at 2160-2688, and 6 GiB at 3072. Higher existing allowances are preserved. This addresses the observed high-resolution guest-memory allocation failure.
- Candidate fixes for missed CPU-write tracking on empty GPU readbacks and dangling references in deferred buffer readback callbacks. These target stale texture data and potential crashes; visual/stability improvements from these latest changes are not yet verified.

Desktop mirror options are 60 FPS, 30 FPS and uncapped, independently of the headset. Change headset refresh and motion-smoothing settings in your OpenXR runtime.

## Controls

Right A: Cross; right B: Square; left X/Y: Circle/Triangle. Left stick moves, right stick turns; triggers are L2/R2, grips L1/R1, left menu Options. Both stick clicks recenter the host view. The right controller supplies the experimental Aim pose. Complete the game's initial calibration.

## Validation and known issues

User testing confirmed improved brightness and successful operation after increasing guest memory. A captured run at 3072x3456 per eye reported approximately 89.6-90 game submissions per second on a 90 Hz headset. This is one scene, not a campaign-wide performance guarantee.

Rainbow patterns on distant gourd textures and large stretched triangles near the volcano remain under investigation. Captures show corrupted texture mips; the newest cache/readback candidate still needs gameplay confirmation. A separate GPU-thread access violation is also under investigation; improved crash logging is included. A complete campaign, every interaction, audio and save compatibility have not been validated.

Build, compatibility/protected-page file-read tests and texture-subresource regression tests are available. Diagnostic code is opt-in through local request files; no shader overrides or diagnostic requests are shipped in the release.

## Source and build

The `farpoint-pc-vr` branch derives from AstroQuest commit `3a75299ff5216f3f255a987d462cc974fcba9eb3`. Initialize upstream submodules, then configure `shadps4-arm64-main` with Ninja, a Windows clang-cl toolchain and SDK, Release mode, `ENABLE_OPENXR=ON`, `ENABLE_UPDATER=OFF`, `ENABLE_DISCORD_RPC=OFF`, and `ALSOFT_UPDATE_BUILD_VERSION=OFF`. `tools/build-farpoint.ps1` supports the portable toolchain layout used locally. Run it with `-Test` for compatibility and protected-read checks; `tools/test-image-subresources.ps1` checks the production subresource methods.

Packed fragment-export handling adapts the proposal by artemkaVG in upstream shadPS4 PR #5279: https://github.com/shadps4-emu/shadPS4/pull/5279. Other inherited components retain their licenses and attribution; see `LICENSE`, `THIRD-PARTY-NOTICES.md` and `.gitmodules`.

No game packages, executables, shaders, extracted configuration packs, saves or private captures belong in Git or public release assets.
