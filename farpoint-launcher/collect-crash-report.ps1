$ErrorActionPreference = 'Stop'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$report = Join-Path $PSScriptRoot "reports\crash-$stamp"
New-Item -ItemType Directory -Path $report -Force | Out-Null
foreach ($name in @('BUILD.txt','farpoint-settings.json','user/config.json','user/log')) {
    $path = Join-Path $PSScriptRoot $name
    if (Test-Path -LiteralPath $path) { Copy-Item -LiteralPath $path -Destination $report -Recurse }
}
Get-FileHash (Join-Path $PSScriptRoot 'shadps4.exe') -Algorithm SHA256 | Format-List | Out-File (Join-Path $report 'executable-hash.txt')
Compress-Archive -LiteralPath $report -DestinationPath "$report.zip"
Write-Host "Report saved: $report.zip"
Write-Host 'Review before sharing: logs and settings may contain local paths.'
