param([string]$ToolchainRoot, [string]$SourcePath)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
if (-not $ToolchainRoot) { $ToolchainRoot=Join-Path $root ../toolchain }
$tc=(Resolve-Path $ToolchainRoot).Path
$crt="$tc/winsysroot/VC/Tools/MSVC/14.44.17.14"
$sdk="$tc/winsysroot/Windows Kits/10"
$env:INCLUDE="$crt/include;$sdk/Include/10.0.26100/ucrt;$sdk/Include/10.0.26100/um;$sdk/Include/10.0.26100/shared"
$env:LIB="$crt/lib/x64;$sdk/Lib/10.0.26100/um/x64;$sdk/Lib/10.0.26100/ucrt/x64"
$clang="$tc/clang+llvm-21.1.8-x86_64-pc-windows-msvc/bin/clang-cl.exe"
$command=(& "$tc/python-tools/bin/ninja.exe" -C "$root/build/win-fix" -t commands | Select-String ' -c -- .*image_info.cpp$').Line
if (-not $command) { throw 'Configure/build Farpoint before running this test.' }
$includes=@([regex]::Matches($command,'-(?:I|imsvc)(\S+)') | ForEach-Object { '/I'+$_.Groups[1].Value })
$out="$root/build/compat-tests"
# Compile the actual production methods without pulling in unrelated constructors
# and the entire emulator. Do not maintain a separate implementation in this test.
if (-not $SourcePath) { $SourcePath="$root/shadps4-arm64-main/src/video_core/texture_cache/image_info.cpp" }
$source=Get-Content $SourcePath -Raw
$compatStart=$source.IndexOf('bool ImageInfo::IsCompatible(')
$compatEnd=$source.IndexOf('void ImageInfo::UpdateSize()', $compatStart)
$mipStart=$source.IndexOf('s32 ImageInfo::MipOf(')
$mipEnd=$source.LastIndexOf('} // namespace VideoCore')
$isolated='#include "video_core/amdgpu/resource.h"' + "`n" + '#include "video_core/texture_cache/image_info.h"' + "`n" + '#include "video_core/texture_cache/host_compatibility.h"' + "`nnamespace VideoCore {`n" + $source.Substring($compatStart,$compatEnd-$compatStart) + $source.Substring($mipStart,$mipEnd-$mipStart) + "`n}"
Set-Content "$out/image_subresource_production.cpp" $isolated
& $clang /nologo /std:c++latest /MD /EHsc /O2 /Gy /clang:-flto=thin /DNOMINMAX /DWIN32_LEAN_AND_MEAN -fuse-ld=lld @includes "$root/tools/tests/image_subresource_test.cpp" "$out/image_subresource_production.cpp" "$root/shadps4-arm64-main/src/video_core/texture_cache/host_compatibility.cpp" "/Fo:$out/" "/Fe:$out/image_subresource_test.exe" /link /OPT:REF
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& "$out/image_subresource_test.exe"
exit $LASTEXITCODE

