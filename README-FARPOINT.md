# Farpoint PC VR development build

Farpoint CUSA04508 version 1.00 running through AstroQuest's shadPS4/OpenXR host on Windows. This is a development port, with known rendering defects and incomplete campaign validation.

## First-time installation

1. Extract the release ZIP into a writable folder, for example `D:\Farpoint`.
2. Place your own **Farpoint CUSA04508 version 1.00 package** in that folder and name it **`Farpoint.pkg`**, beside **`Play Farpoint.bat`** (not inside `pc-vr`).
3. Connect your VR headset and activate its OpenXR runtime, such as SteamVR or Virtual Desktop's VDXR.
4. Open **`Play Farpoint.bat`**, choose your resolution and controller options, leave the default game path selected, and click Start Farpoint. The first launch extracts the package, creates the emulator settings and generates the resolution profiles from your game. Leave the console open until extraction finishes.

Example folder layout before the first launch:

```text
D:\Farpoint\
  Farpoint.pkg       <-- your game package goes here
  Play Farpoint.bat  <-- open this
  pc-vr\
    shadps4.exe
    launch-farpoint.ps1
    initialize-farpoint.ps1
    FarpointProfiles.cs
    config-default.json
    pkgtool\
      PkgTool.exe
      LibOrbisPkg.dll
```

Alternatively, supply an already extracted game with its complete directory structure at `games/CUSA04508/files/uroot/eboot.bin`, or use Browse to select that executable elsewhere. Do not copy only `eboot.bin`; the entire extracted game is required.

Requires 64-bit Windows, a Vulkan-capable GPU, an OpenXR headset/runtime, Windows PowerShell, and the [Microsoft Visual C++ x64 runtime](https://aka.ms/vs/17/release/vc_redist.x64.exe). Allow disk space for both the package and extracted game. Package extraction supports the compatible zero-passcode package format; unsupported packages are rejected. No game files are provided. Existing settings are preserved.

The ZIP includes the emulator, launcher, setup code, default configuration, package-extraction tool and licenses. Resolution packs are generated locally from your game on first launch, so no separate profile download is needed.

Supported game executable SHA-256: `92A21FF9E309CE5DD58B058C4B12329463697BB27FDA98B5DA353E0A73BD9C0F`.

## Features

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
