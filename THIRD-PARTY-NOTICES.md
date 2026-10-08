# Third-party notices

Farpoint PC VR is derived from AstroQuest and shadPS4 and is licensed under GPL-2.0-or-later; see LICENSE and copyright notices in the source. Upstream authorship and Git history are retained.

## Windows PC build

| Component | License | Source |
| --- | --- | --- |
| Farpoint emulator and launcher | GPL-2.0-or-later | This repository at the release tag |
| AstroQuest foundation | GPL-2.0-or-later | https://github.com/bigmak94/AstroQuest |
| shadPS4 and shadps4-arm64 foundation | GPL-2.0-or-later | https://github.com/shadps4-emu/shadPS4 and https://github.com/zenithblue-oss/shadps4-arm64 |
| Linked dependencies, including SDL3, fmt, Boost, glslang, sirit, Vulkan Memory Allocator, zlib-ng, xxHash and FFmpeg components | Individual licenses preserved in each dependency | Exact submodule revisions under shadps4-arm64-main/externals; see .gitmodules |
| Khronos OpenXR loader | Apache-2.0 | shadps4-arm64-main/externals/openxr-sdk; https://github.com/KhronosGroup/OpenXR-SDK |

Packed fragment-export handling adapts artemkaVG's upstream shadPS4 proposal https://github.com/shadps4-emu/shadPS4/pull/5279. The inherited performance work is documented in source history, including changes adapted from elliotttate's AstroQuest fork.

The emulator requires the Microsoft Visual C++ runtime, which is not included: https://aka.ms/vs/17/release/vc_redist.x64.exe.

## Package extraction

PkgTool and LibOrbisPkg 0.2.231 by Maxton are bundled unchanged under LGPL-3.0. Their license and source information are included in `pc-vr/pkgtool/`. Source: https://github.com/maxton/LibOrbisPkg (release v0.2).

## Corresponding source

Emulator and launcher source is available at each release tag. Exact dependency revisions and upstream URLs are recorded in .gitmodules and the submodule entries. The inherited offer to provide the corresponding source of GPL/LGPL components on request for three years from each release remains applicable; open an issue in this repository.

## Not included

No game package, game executable, extracted configuration pack, game shader, save data, PlayStation system software or console key is included in this repository or its releases. The Windows distribution does not include a Quest/Android application or Linux runtime.
