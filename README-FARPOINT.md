# Farpoint PC VR v0.4.0 — Newer shadPS4 Comparison Prerelease

This experimental build uses shadPS4 baseline `06e813ff8c8e2a6d81b02cabfcc9543c3debf78e` with the Farpoint OpenXR port. Install in a separate folder for comparison with the earlier build.

## Included

- OpenXR headset rendering, tracked Aim controls and the resolution launcher, through 3072 × 3456 per eye.
- Virtual Desktop exception fix: normal first-chance C++ exceptions can reach their native handlers instead of prematurely shutting down the emulator.
- Defensive validation of invalid time-query output addresses.
- Expanded fatal crash reports: CPU registers, stack/red-zone contents, code bytes, memory mappings, frame-chain candidates and host-module backtraces.
- Per-session executable hashes, automatic preservation of the previous log, and a crash-report ZIP collector.
- Guest red-zone protection and the newer renderer's precise readback configuration enabled for this comparison.

## First-time installation

1. Download **Farpoint-PC-VR-v0.4.0-First-Install.zip** and extract it to a new writable folder.
2. Put your own **Farpoint CUSA04508 version 1.00** package in that root folder, beside **Play Farpoint.bat**, and name it **Farpoint.pkg**. Do not put it inside `pc-vr`.
3. Connect your headset using your OpenXR runtime, such as SteamVR or Virtual Desktop VDXR.
4. Open **Play Farpoint.bat**, choose your settings and start. First launch extracts the compatible zero-passcode package, restores metadata and generates resolution profiles. Keep the console open until setup finishes.

You may instead browse to the supported `eboot.bin` in a complete extracted game. The executable SHA-256 must be `92A21FF9E309CE5DD58B058C4B12329463697BB27FDA98B5DA353E0A73BD9C0F`. Other game versions are not verified.

Requires 64-bit Windows, a Vulkan-capable GPU, an OpenXR headset/runtime and the [Microsoft Visual C++ x64 runtime](https://aka.ms/vs/17/release/vc_redist.x64.exe). The ZIP includes the emulator, launcher, extraction/setup tools, configuration and licenses. No game files, generated game profiles, saves or personal logs are included.

## Controls

Right A/B: Cross/Square; left X/Y: Circle/Triangle. Left stick moves; right stick turns. Triggers: L2/R2; grips: L1/R1; left menu: Options. Both stick clicks recenter. The right controller supplies the Aim pose. Complete the game's calibration.

## Report a crash

After a crash, right-click **pc-vr/collect-crash-report.ps1** and select **Run with PowerShell**. Attach the ZIP created under `pc-vr/reports` to an [issue](https://github.com/evertec82/Farpoint/issues). Include the level/action, headset/runtime, GPU/driver and whether the older build also crashes. Review the report before sharing because logs/settings can contain local paths. No continuous image capture or hardware watchpoints are enabled.

## Testing limitations

Both compilation and executable/launcher smoke checks passed. Headset gameplay, campaign stability and rendering correctness have **not** been established for this new baseline. Its renderer/readback implementation differs from the earlier port; not all old title-specific rendering workarounds were carried over, so lighting or geometry may regress. Existing stretched geometry, texture artifacts and render-thread crashes are not claimed fixed. This prerelease is for comparison and collecting actionable crash reports.

The source for this candidate is on the `farpoint-upstream-preview` branch. See `BUILD.txt` for the exact source and executable identity, and `BUILD-FARPOINT.md` in the source for build instructions.

## Version history

- v0.1.0: First-time installation / metadata fix.
- v0.2.0: Stability improvements.
- v0.3.0: Crash reporting and time-query guard.
- v0.4.0: Newer shadPS4 comparison, Virtual Desktop fix and expanded diagnostics.

Versions increase with each published revision. Existing release URLs are retained.
