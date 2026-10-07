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
if ($Test) {
    $testDir = Join-Path $repoRoot 'build/compat-tests'
    New-Item -ItemType Directory -Force $testDir | Out-Null
    $sourceRoot = Join-Path $repoRoot 'shadps4-arm64-main'
    $testsRoot = Join-Path $repoRoot 'tools/tests'
    & "$llvmBin/clang-cl.exe" /nologo /std:c++latest /MD /EHsc -fuse-ld=lld "/I$testsRoot/stubs" "/I$sourceRoot/src" "$testsRoot/farpoint_compat_test.cpp" "$sourceRoot/src/core/libraries/libc_internal/libc_internal_cxa.cpp" "/Fe:$testDir/farpoint_compat_test.exe" "/Fo:$testDir/"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & "$testDir/farpoint_compat_test.exe"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & "$llvmBin/clang-cl.exe" /nologo /std:c++latest /MD /EHsc -fuse-ld=lld "/I$sourceRoot/src" "$testsRoot/staged_file_read_test.cpp" "/Fe:$testDir/staged_file_read_test.exe" "/Fo:$testDir/"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & "$testDir/staged_file_read_test.exe"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    Write-Output 'Farpoint compatibility and protected-page file read tests passed'
    exit 0
}
$buildDir = Join-Path $repoRoot 'build/win-fix'
if ($Configure) {
    & $cmakeExe -S (Join-Path $repoRoot 'shadps4-arm64-main') -B $buildDir -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninjaExe" "-DCMAKE_C_COMPILER=$llvmBin/clang-cl.exe" "-DCMAKE_CXX_COMPILER=$llvmBin/clang-cl.exe" "-DCMAKE_RC_COMPILER=$llvmBin/llvm-rc.exe" '-DCMAKE_BUILD_TYPE=Release' '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL' '-DENABLE_DISCORD_RPC=OFF' '-DENABLE_UPDATER=OFF' '-DENABLE_OPENXR=ON' '-DALSOFT_UPDATE_BUILD_VERSION=OFF'
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
& $cmakeExe --build $buildDir --target shadps4 --parallel 8
exit $LASTEXITCODE
