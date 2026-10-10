# Building the Farpoint upstream preview

Baseline: shadPS4 06e813ff8c8e2a6d81b02cabfcc9543c3debf78e. VR bridge adapted from the earlier Farpoint/AstroQuest port; inherited source/license notices are retained.

Initialize the pinned submodules with `git submodule update --init --recursive`. OpenXR SDK source is included under externals/openxr-sdk. Configure the repository root with CMake/Ninja, Windows clang-cl 21 and a Windows SDK, Release mode, ENABLE_OPENXR=ON, ENABLE_DISCORD_RPC=OFF, ENABLE_UPDATER=OFF, ALSOFT_UPDATE_BUILD_VERSION=OFF; then build the shadps4 target. Supply Python3_EXECUTABLE for your installed Python. tools/build-prototype.ps1 records the portable toolchain layout used for this candidate; adjust local Python/toolchain locations to your machine.

Launcher/setup source is in farpoint-launcher. Package extraction uses the GPL LibOrbisPkg/PkgTool component and attribution/source link in the binary package's pc-vr/pkgtool/README.txt. Generated profiles, game packages and saves are not part of the source.

This is a comparison prerelease; no gameplay stability or correctness guarantee is implied. See README-FARPOINT.md.
