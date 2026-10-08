# Farpoint PC VR

A Windows PC VR development port of Farpoint using shadPS4 and OpenXR, based on [AstroQuest](https://github.com/bigmak94/AstroQuest).

- [Download Farpoint PC VR](https://github.com/evertec82/Farpoint/releases)
- [Setup, controls, resolutions, known issues and build instructions](README-FARPOINT.md)
- [License](LICENSE) and [third-party notices](THIRD-PARTY-NOTICES.md)

Current work includes corrected lighting, tracked controllers, SteamVR/OpenXR support, full-refresh frame pacing, and supersampling through 3072 x 3456 per eye.

**This is a development port.** Rainbow textures, stretched geometry and intermittent crashes remain under investigation. The downloadable ZIP supports first-time installation. Place your own compatible game package, named `Farpoint.pkg`, beside `Play Farpoint.bat`, then open the launcher. Extraction, configuration and resolution profiles are prepared automatically. The game is not included.

This repository preserves the upstream AstroQuest/shadPS4 source history and attribution. Farpoint changes are on `main`. Shared emulator code and upstream license notices are retained because the Farpoint build depends on them.
