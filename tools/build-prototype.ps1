param([string]$ToolchainRoot, [switch]$Configure, [switch]$Test)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
if (-not $ToolchainRoot) { $ToolchainRoot = Join-Path $repoRoot '../toolchain' }
$llvmBin = Join-Path $ToolchainRoot 'clang+llvm-21.1.8-x86_64-pc-windows-msvc/bin'
$llvmBin = $llvmBin.Replace('\', '/')
$cmakeExe = Join-Path $ToolchainRoot 'python-tools/cmake/data/bin/cmake.exe'
$ninjaExe = Join-Path $ToolchainRoot 'python-tools/bin/ninja.exe'
$crtRoot = Join-Path $ToolchainRoot 'winsysroot/VC/Tools/MSVC/14.44.17.14'
$sdkRoot = Join-Path $ToolchainRoot 'winsysroot/Windows Kits/10'
$sdkInclude = Join-Path $sdkRoot 'Include/10.0.26100'
$sdkLib = Join-Path $sdkRoot 'Lib/10.0.26100'
$env:INCLUDE = "$crtRoot/include;$sdkInclude/ucrt;$sdkInclude/um;$sdkInclude/shared;$sdkInclude/winrt;$sdkInclude/cppwinrt"
$env:LIB = "$crtRoot/lib/x64;$sdkLib/um/x64;$sdkLib/ucrt/x64"
$env:PATH = "$llvmBin;$(Split-Path $ninjaExe);$env:PATH"
$buildDir = Join-Path $repoRoot 'build-upstream'
& $cmakeExe -S $repoRoot -B $buildDir -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninjaExe" "-DCMAKE_C_COMPILER=$llvmBin/clang-cl.exe" "-DCMAKE_CXX_COMPILER=$llvmBin/clang-cl.exe" "-DCMAKE_RC_COMPILER=$llvmBin/llvm-rc.exe" '-DCMAKE_BUILD_TYPE=Release' '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL' '-DENABLE_DISCORD_RPC=OFF' '-DENABLE_UPDATER=OFF' '-DENABLE_OPENXR=ON' '-DPython3_EXECUTABLE=C:/Users/jonat/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' '-DALSOFT_UPDATE_BUILD_VERSION=OFF'
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $cmakeExe --build $buildDir --target shadps4 --parallel 8 -- -k 0
exit $LASTEXITCODE