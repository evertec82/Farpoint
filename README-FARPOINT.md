Farpoint PC VR v0.9.5 improves CPU overhead, shader-cache reuse and frame pacing. This remains an experimental prerelease.

### Changes
- Prewarms cached graphics pipelines on the compilation worker before gameplay and drains pending cache writes on normal shutdown. Draws still wait for required pipelines; no objects or effects are deliberately skipped. Previously unseen shaders can still cause first-use stutters.
- Reduces repeated texture lookup, shader-read barrier bookkeeping and GPU completion polling.
- Batches texture readbacks and submits GPU work earlier while retaining required readback completion and resource-lifetime synchronization.
- Corrects display-clock timeout handling and reduces routine memory logging.
- Coalesces adjacent release ranges, avoids copying memory-map records during readbacks, and clears stale GPU-dirty state after empty readbacks.
- Adds targeted memory-release, readback and OpenXR stall timings to aid diagnosis. Crash-report ZIP collection remains included.
- Adds 3456 x 3888 and 3840 x 4320 per-eye resolution presets.
- Retains the gamma, dithering, haptics, FOV, controller mapping and existing lighting/readback fixes.

### Validation and remaining limitations
Windows build and isolated startup/rendering/shutdown checks passed. Recent local headset runs reached approximately 90 FPS in smooth sections. A previously observed 199.5 ms gameplay pipeline wait was handled during prewarming on the next run, with no gameplay compilation waits recorded in that sample. These are observations from different runs, not a controlled benchmark.

Memory/resource-release stalls remain under investigation. The latest cleanup changes passed build and startup/shutdown checks but have not yet been measured in headset gameplay. They do not guarantee elimination of stalls or rendering issues. Long-session stability, lighting and texture correctness still need gameplay validation.

### First-time installation
1. Download **Farpoint-PC-VR-v0.9.5-First-Install.zip** and extract to a writable folder.
2. Put your own supported **Farpoint CUSA04508 version 1.00** package beside **Play Farpoint.bat**, named **Farpoint.pkg**, or browse to `eboot.bin` in your complete extracted game.
3. Start your OpenXR runtime and open **Play Farpoint.bat**. First launch performs extraction/setup. Keep the console open until setup finishes.

The ZIP includes the emulator, launcher, setup tools, default configuration and licenses. No game files, generated game profiles, saves or personal logs are included. Requires 64-bit Windows, a Vulkan-capable GPU, OpenXR and the Microsoft Visual C++ x64 runtime.

For an existing installation, extract into a new folder and select your existing game. Copy `pc-vr/user/home/1000/savedata/CUSA04508` into the corresponding folder to retain your save. Back up custom game overrides before setup regenerates its profiles.

### Comparison and reports
Set `SHADPS4_ASYNC_PIPELINES=0` in the environment before launching to compare synchronous compilation. Set `SHADPS4_OUTPUT_DITHER=0` to compare without dithering. Restart after changes. The launcher offers 30/60 FPS mirror choices.

After a crash, run `pc-vr/collect-crash-report.ps1` and attach its ZIP to an issue, with the scene, headset/runtime, GPU and resolution. Green/purple textures, stretched geometry, lighting issues and other crashes may remain.
