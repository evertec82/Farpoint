param([Parameter(Mandatory=$true)][string]$GamePath, [string]$Package)
$ErrorActionPreference='Stop'
$tool=Join-Path $PSScriptRoot 'pkgtool/PkgTool.exe'
$sys=Join-Path (Split-Path $GamePath -Parent) 'sce_sys'
$sfo=Join-Path $sys 'param.sfo'
if (-not (Test-Path -LiteralPath $sfo)) {
    if (-not $Package -or -not (Test-Path -LiteralPath $Package)) { throw 'Missing sce_sys/param.sfo. Place the original Farpoint.pkg beside Play Farpoint.bat to repair the installation, or restore the complete sce_sys directory from the same game package.' }
    $entries=@(& $tool pkg_listentries $Package)
    if ($LASTEXITCODE -ne 0) { throw 'Could not read game package metadata.' }
    # These entries live outside the package filesystem and pkg_extract omits them.
    foreach ($line in $entries) {
        if ($line -notmatch '^0x[0-9A-Fa-f]+\s+0x[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+(\d+)\s+(?:\d+\s+)?([A-Z0-9_]+)\s*$') { continue }
        $index=$Matches[1]; $name=$Matches[2]
        $relative=switch -Regex ($name) {
            '^PARAM_SFO$' { 'param.sfo'; break }
            '^ICON0_PNG$' { 'icon0.png'; break }
            '^PIC1_PNG$' { 'pic1.png'; break }
            '^NPBIND_DAT$' { 'npbind.dat'; break }
            '^NPTITLE_DAT$' { 'nptitle.dat'; break }
            '^TROPHY__([A-Z0-9_]+)_TRP$' { 'trophy/'+$Matches[1].ToLower()+'.trp'; break }
            default { $null }
        }
        if (-not $relative) { continue }
        $destination=Join-Path $sys $relative
        if (Test-Path -LiteralPath $destination) { continue }
        New-Item -ItemType Directory -Path (Split-Path $destination -Parent) -Force | Out-Null
        & $tool pkg_extractentry --passcode '00000000000000000000000000000000' $Package $index ($destination+'.tmp') | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "Could not extract $relative from the game package." }
        Move-Item -LiteralPath ($destination+'.tmp') -Destination $destination
    }
}
if (-not (Test-Path -LiteralPath $sfo)) { throw 'The game package did not supply sce_sys/param.sfo.' }
$details=@(& $tool sfo_listentries $sfo)
if ($LASTEXITCODE -ne 0 -or -not ($details -match '^TITLE_ID .* = CUSA04508$') -or -not ($details -match '^APP_VER .* = 01\.00$')) { throw 'Game metadata must identify the supported Farpoint CUSA04508 version 1.00. Do not substitute metadata from another game or edition.' }
