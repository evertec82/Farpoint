param([Parameter(Mandatory=$true)][string]$GamePath)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
if (-not (Test-Path -LiteralPath $GamePath)) {
    $package=Join-Path $root 'Farpoint.pkg'
    $defaultGame=Join-Path $root 'games/CUSA04508/files/uroot/eboot.bin'
    if ([IO.Path]::GetFullPath($GamePath) -ne [IO.Path]::GetFullPath($defaultGame) -or -not (Test-Path -LiteralPath $package)) { throw 'Place Farpoint.pkg beside Play Farpoint.bat, or browse to an extracted eboot.bin.' }
    $target=Join-Path $root 'games/CUSA04508'
    $staging=Join-Path $root ('games/.farpoint-extract-'+[guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $staging -Force | Out-Null
    Write-Host 'Extracting Farpoint.pkg. This can take several minutes. Keep this window open.'
    & (Join-Path $PSScriptRoot 'pkgtool/PkgTool.exe') pkg_extract --passcode '00000000000000000000000000000000' $package (Join-Path $staging 'files')
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path (Join-Path $staging 'files/uroot/eboot.bin'))) { throw "Package extraction failed. Partial files retained at $staging. Supply a compatible CUSA04508 1.00 package or an extracted game." }
    if (Test-Path -LiteralPath $target) { throw "Destination already exists: $target. Extracted files are at $staging; no existing files were replaced." }
    $gamesRoot=[IO.Path]::GetFullPath((Join-Path $root 'games'))+[IO.Path]::DirectorySeparatorChar
    foreach ($candidate in @($staging,$target)) {
        if (-not [IO.Path]::GetFullPath($candidate).StartsWith($gamesRoot,[StringComparison]::OrdinalIgnoreCase)) { throw 'Extraction destination is outside the games folder.' }
    }
    Move-Item -LiteralPath $staging -Destination $target
}
if ((Get-FileHash -LiteralPath $GamePath).Hash -ne '92A21FF9E309CE5DD58B058C4B12329463697BB27FDA98B5DA353E0A73BD9C0F') { throw 'Only Farpoint CUSA04508 version 1.00 is supported.' }
$profiles=Join-Path $PSScriptRoot 'resolution-profiles'
$missing=@(960,1536,1920,2160,2400,2688,3072) | Where-Object { -not (Test-Path (Join-Path $profiles "eye-$_.pak")) }
if ($missing) {
    if (-not ('FarpointProfiles' -as [type])) { Add-Type -Path (Join-Path $PSScriptRoot 'FarpointProfiles.cs') }
    [FarpointProfiles]::Generate((Join-Path (Split-Path $GamePath -Parent) 'refuge/content/paks/pakchunk0-ps4.pak'), $profiles)
    Write-Host 'Created resolution profiles from your game.'
}
$userDir=Join-Path $PSScriptRoot 'user'
New-Item -ItemType Directory $userDir -Force | Out-Null
if (-not (Test-Path (Join-Path $userDir 'config.json'))) { Copy-Item (Join-Path $PSScriptRoot 'config-default.json') (Join-Path $userDir 'config.json') }
