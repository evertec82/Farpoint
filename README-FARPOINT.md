# Farpoint PC VR v0.7.0

This experimental PC VR release uses shadPS4 baseline `06e813ff8c8e2a6d81b02cabfcc9543c3debf78e` with the Farpoint OpenXR port.

## Changes

- Raises the game graphics-memory pool to 3,072 MiB. Diagnostics found fragmentation preventing a 4 MiB allocation in the previous 2,400 MiB pool. A local session exceeded the old budget without that crash; this is a mitigation, not a guarantee of stability.
- Limits single-pixel GPU readbacks to the actual pixel rather than copying unwritten row padding into guest memory. Green textures and overly bright lighting remain under investigation; this fix needs further gameplay validation.
- Applies a tone-mapper gamma override of 2.0 to darken the image. This is a visual adjustment, not a correction for every lighting artifact.
- Left thumbstick click toggles 3x movement speed; click again for normal speed. Both stick clicks still recenter and clear the toggle. Controller/focus loss also clears it.
- Adjusts the right-hand Aim pose downward by 20 degrees and forward by approximately 7 inches.
- Allows headset rendering to proceed when a desktop mirror frame is unavailable, reduces presentation-log noise, and retains full-refresh pacing. Performance improvement is not guaranteed.
- Adds direct fault-context files and a bounded history of the latest 4,096 memory-allocation events for supported crash paths, plus GPU-wait timing summaries. Texture-source capture is disabled by the launcher. The developer's external Python pool monitor is not included or required.
- Retains stack protection, spatial-audio corrections, Virtual Desktop compatibility, SteamVR/OpenXR support, and resolution choices through 3072 x 3456 per eye.

Game-specific configuration and sprint overrides are generated locally from the supported game during first setup. The ZIP contains no game assets, generated game profiles, saves, personal logs, or diagnostic captures.

## First-time installation

1. Download **Farpoint-PC-VR-v0.7.0-First-Install.zip** and extract it to a new writable folder.
2. Put your own **Farpoint CUSA04508 version 1.00** package in that root folder, beside **Play Farpoint.bat**, and name it **Farpoint.pkg**. Do not put it inside `pc-vr`.
3. Connect your headset using your OpenXR runtime, such as SteamVR or Virtual Desktop VDXR.
4. Open **Play Farpoint.bat**, choose your settings and start. First launch extracts the compatible zero-passcode package, restores metadata and generates resolution profiles. Keep the console open until setup finishes.

You may instead browse to the supported `eboot.bin` in a complete extracted game. The executable SHA-256 must be `92A21FF9E309CE5DD58B058C4B12329463697BB27FDA98B5DA353E0A73BD9C0F`. Other game versions are not verified.

Requires 64-bit Windows, a Vulkan-capable GPU, an OpenXR headset/runtime and the [Microsoft Visual C++ x64 runtime](https://aka.ms/vs/17/release/vc_redist.x64.exe). The ZIP includes the emulator, launcher, extraction/setup tools, configuration and licenses. No game files, generated game profiles, saves or personal logs are included.

## Controls

Right A/B: Cross/Square; left X/Y: Circle/Triangle. Left stick moves; right stick turns. Triggers: L2/R2; grips: L1/R1; left menu: Options. Left stick click toggles 3x sprint. Both stick clicks recenter. The right controller supplies the Aim pose. Complete the game's calibration.

## Existing installations

Close the game and extract to a new folder. Select your existing complete extracted game's `eboot.bin` in the launcher, or supply `Farpoint.pkg` as described above. To retain progress, copy your existing `pc-vr/user/savedata` folder into the new `pc-vr/user` folder before starting. Setup regenerates the sprint and memory/gamma overrides beside that game's original pak files. Back up any custom overrides before using the same game directory.

## Report a crash

After a crash, right-click `pc-vr/collect-crash-report.ps1` and choose **Run with PowerShell**. Attach the resulting ZIP under `pc-vr/reports` to an issue at https://github.com/evertec82/Farpoint/issues. Include the level/action, headset/runtime, GPU/driver and chosen resolution. Review reports before sharing; logs can contain local paths. Some abnormal exits still do not produce a fault-context file.

## Validation and known issues

The emulator builds successfully. Launcher syntax, generated memory/gamma and sprint pack parity with the local candidates, and package integrity were checked. The latest gamma and single-pixel readback changes have not yet been confirmed in headset gameplay. Green/purple textures, excessive brightness, stretched geometry, performance limitations, and additional crashes may remain. This release remains a prerelease for testing and reports.

Source is on `farpoint-upstream-preview`; see `BUILD.txt` in the download for the exact source and executable identity and `BUILD-FARPOINT.md` in the repository for build instructions.
