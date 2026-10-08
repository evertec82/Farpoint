# Reporting Farpoint crashes

This build includes a defensive check for invalid time-query output addresses and expanded crash-only logging. It does not resolve all crashes. Normal gameplay does not trigger image, shader or geometry captures.

After a crash, copy `pc-vr/user/log/shad_log.txt` somewhere safe **before starting the game again**. If Windows offers to close the program, let it finish closing first. If no fatal error appears in the log, still report the crash and include any console exit code.

Open an issue at https://github.com/evertec82/Farpoint/issues and attach the saved log (or a ZIP of it). Include:

- Release tag and the contents of `BUILD.txt` from the extracted download.
- Game title ID, version and region.
- GPU, driver version, headset and OpenXR runtime (SteamVR or Virtual Desktop).
- Resolution preset, frame-rate selection and whether settings changed during the session.
- Level, nearby enemy or object, action immediately before the crash, and whether it repeats.
- Any launcher error or exit code.

Logs contain local paths and machine details; review them before posting publicly. Do not attach game packages, executables, saves or memory dumps. The enhanced fatal report includes CPU registers and stack words, but does not capture images or full memory dumps.

The time-query check prevents writes to impossible address ranges; it does not validate every mapped pointer or explain the source of corrupt pointers. Repeated render-thread and GPU callback failures, stretched geometry and texture artifacts remain under investigation. A successful short run does not establish that those issues are fixed.
