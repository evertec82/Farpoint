Farpoint PC VR v0.9.6 raises the default game-side gamma from 1.2 to 1.4 for moderately brighter shadow detail. All v0.9.5 performance, cache, controller and diagnostic changes are retained.

The launcher regenerates the gamma override on first launch. This is a brightness adjustment, not a confirmed fix for missing indirect lighting or texture detail in shadows. This remains a prerelease.

### First-time installation
1. Download **Farpoint-PC-VR-v0.9.6-First-Install.zip** and extract to a writable folder.
2. Put your own supported **Farpoint CUSA04508 version 1.00** package beside **Play Farpoint.bat**, named **Farpoint.pkg**, or browse to `eboot.bin` in your complete extracted game.
3. Start your OpenXR runtime and open **Play Farpoint.bat**. First launch performs extraction/setup. Keep the console open until setup finishes.

The ZIP includes the emulator, launcher, setup tools, default configuration and licenses. No game files, generated game profiles, saves or personal logs are included. Requires 64-bit Windows, a Vulkan-capable GPU, OpenXR and the Microsoft Visual C++ x64 runtime.

For an existing installation, extract into a new folder and select your existing game. Copy `pc-vr/user/home/1000/savedata/CUSA04508` into the corresponding folder to retain your save. Back up custom game overrides before setup regenerates its profiles.

### Comparison and reports
Set `SHADPS4_ASYNC_PIPELINES=0` in the environment before launching to compare synchronous compilation. Set `SHADPS4_OUTPUT_DITHER=0` to compare without dithering. Restart after changes. The launcher offers 30/60 FPS mirror choices.

After a crash, run `pc-vr/collect-crash-report.ps1` and attach its ZIP to an issue, with the scene, headset/runtime, GPU and resolution. Green/purple textures, stretched geometry, lighting issues and other crashes may remain.
