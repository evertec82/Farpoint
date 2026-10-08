param(
    [string]$GamePath,
    [ValidateSet('Aim','Gamepad')][string]$Controller = 'Aim',
    [ValidateSet(0,30,60)][int]$MirrorFps = 60,
    [ValidateSet(960,1536,1920,2160,2400,2688,3072)][int]$EyeWidth = 1536,
    [switch]$NoGui,
    [switch]$ValidateOnly
)
$ErrorActionPreference = 'Stop'
$installRoot = Split-Path $PSScriptRoot -Parent
$exe = Join-Path $PSScriptRoot 'shadps4.exe'
$settingsFile = Join-Path $PSScriptRoot 'farpoint-settings.json'
if (-not $GamePath) {
    $GamePath = Join-Path $installRoot 'games/CUSA04508/files/uroot/eboot.bin'
    if (Test-Path $settingsFile) {
        $saved = Get-Content $settingsFile -Raw | ConvertFrom-Json
        if ($saved.GamePath) { $GamePath = $saved.GamePath }
        if (-not $PSBoundParameters.ContainsKey('Controller') -and $saved.Controller -in @('Aim','Gamepad')) { $Controller = $saved.Controller }
        if (-not $PSBoundParameters.ContainsKey('MirrorFps') -and $saved.MirrorFps -in @(0,30,60)) { $MirrorFps = [int]$saved.MirrorFps }
        if (-not $PSBoundParameters.ContainsKey('EyeWidth') -and $saved.EyeWidth -in @(960,1536,1920,2160,2400,2688,3072)) { $EyeWidth = [int]$saved.EyeWidth }
    }
}
if (-not $NoGui -and -not $ValidateOnly) {
    Add-Type -AssemblyName System.Windows.Forms
    Add-Type -AssemblyName System.Drawing
    [System.Windows.Forms.Application]::EnableVisualStyles()
    $form = New-Object System.Windows.Forms.Form
    $form.Text = 'Farpoint PC VR - Development build'
    $form.ClientSize = New-Object System.Drawing.Size(620,365)
    $form.StartPosition = 'CenterScreen'
    $form.FormBorderStyle = 'FixedDialog'
    $form.MaximizeBox = $false
    $label = New-Object System.Windows.Forms.Label
    $label.Text = 'Farpoint 1.00 (CUSA04508). Gameplay and weapon tracking need headset testing.'
    $label.SetBounds(18,16,585,38); $form.Controls.Add($label)
    $pathLabel = New-Object System.Windows.Forms.Label
    $pathLabel.Text = 'Extracted eboot.bin'; $pathLabel.SetBounds(18,60,560,20); $form.Controls.Add($pathLabel)
    $pathBox = New-Object System.Windows.Forms.TextBox
    $pathBox.Text = $GamePath; $pathBox.SetBounds(18,83,490,24); $form.Controls.Add($pathBox)
    $browse = New-Object System.Windows.Forms.Button
    $browse.Text = 'Browse...'; $browse.SetBounds(518,81,83,27); $form.Controls.Add($browse)
    $browse.Add_Click({
        $dialog = New-Object System.Windows.Forms.OpenFileDialog
        $dialog.Filter = 'Game executable (eboot.bin)|eboot.bin'
        if ($dialog.ShowDialog() -eq 'OK') { $pathBox.Text = $dialog.FileName }
        $dialog.Dispose()
    })
    $controlLabel = New-Object System.Windows.Forms.Label
    $controlLabel.Text = 'Controls'; $controlLabel.SetBounds(18,123,270,20); $form.Controls.Add($controlLabel)
    $controls = New-Object System.Windows.Forms.ComboBox
    $controls.DropDownStyle = 'DropDownList'; $controls.SetBounds(18,147,360,25)
    [void]$controls.Items.Add('VR controllers / tracked Aim weapon (experimental)')
    [void]$controls.Items.Add('Gamepad / DualShock mode')
    $controls.SelectedIndex = if ($Controller -eq 'Aim') { 0 } else { 1 }; $form.Controls.Add($controls)
    $mirrorLabel = New-Object System.Windows.Forms.Label
    $mirrorLabel.Text = 'Desktop mirror'; $mirrorLabel.SetBounds(395,123,205,20); $form.Controls.Add($mirrorLabel)
    $mirror = New-Object System.Windows.Forms.ComboBox
    $mirror.DropDownStyle = 'DropDownList'; $mirror.SetBounds(395,147,205,25)
    [void]$mirror.Items.Add('60 FPS'); [void]$mirror.Items.Add('30 FPS'); [void]$mirror.Items.Add('Uncapped')
    $mirror.SelectedIndex = if ($MirrorFps -eq 60) { 0 } elseif ($MirrorFps -eq 30) { 1 } else { 2 }; $form.Controls.Add($mirror)
    $resolutionLabel = New-Object System.Windows.Forms.Label
    $resolutionLabel.Text = 'Headset resolution per eye'; $resolutionLabel.SetBounds(18,187,560,20); $form.Controls.Add($resolutionLabel)
    $resolution = New-Object System.Windows.Forms.ComboBox
    $resolution.DropDownStyle = 'DropDownList'; $resolution.SetBounds(18,211,582,25)
    [void]$resolution.Items.Add('3072 x 3456 per eye - 10.24x pixels (highest)')
    [void]$resolution.Items.Add('2688 x 3024 per eye - 7.84x pixels')
    [void]$resolution.Items.Add('2400 x 2700 per eye - 6.25x pixels')
    [void]$resolution.Items.Add('2160 x 2430 per eye - 5.06x pixels')
    [void]$resolution.Items.Add('1920 x 2160 per eye - 4x pixels')
    [void]$resolution.Items.Add('1536 x 1728 per eye - 2.56x pixels')
    [void]$resolution.Items.Add('Original PSVR - 960 x 1080 per eye')
    $resolution.SelectedIndex = [array]::IndexOf(@(3072,2688,2400,2160,1920,1536,960), $EyeWidth)
    $form.Controls.Add($resolution)
    $info = New-Object System.Windows.Forms.Label
    $info.Text = 'Connect the headset through your active OpenXR runtime before starting. Higher resolution uses the game''s stereo scaling. Choose Original for better performance. The mirror setting does not cap headset frames.'
    $info.SetBounds(18,257,580,45); $form.Controls.Add($info)
    $play = New-Object System.Windows.Forms.Button
    $play.Text = 'Start Farpoint'; $play.SetBounds(455,315,145,32)
    $play.DialogResult = 'OK'; $form.AcceptButton = $play; $form.Controls.Add($play)
    if ($form.ShowDialog() -ne 'OK') { $form.Dispose(); return }
    $GamePath = $pathBox.Text
    $Controller = if ($controls.SelectedIndex -eq 0) { 'Aim' } else { 'Gamepad' }
    $MirrorFps = @(60,30,0)[$mirror.SelectedIndex]
    $EyeWidth = @(3072,2688,2400,2160,1920,1536,960)[$resolution.SelectedIndex]
    $form.Dispose()
}
if (-not (Test-Path -LiteralPath $exe)) { throw "Missing emulator: $exe" }
if (-not (Test-Path -LiteralPath $GamePath)) { throw "Missing extracted game: $GamePath" }
$GamePath = (Resolve-Path -LiteralPath $GamePath).Path
if ([IO.Path]::GetFileName($GamePath) -ne 'eboot.bin') { throw 'Select the extracted Farpoint eboot.bin.' }
# Reject a different executable rather than applying the prototype ABI to another game/version.
$expectedHash = '92A21FF9E309CE5DD58B058C4B12329463697BB27FDA98B5DA353E0A73BD9C0F'
if ((Get-FileHash -LiteralPath $GamePath -Algorithm SHA256).Hash -ne $expectedHash) {
    throw 'This build currently supports only the tested Farpoint CUSA04508 version 1.00 executable.'
}
# Local profiles contain the user's extracted game configuration and are not distributed
# with the source. All retain the existing lighting workaround.
$resolutionProfile = Join-Path $PSScriptRoot ("resolution-profiles/eye-$EyeWidth.pak")
if (-not (Test-Path -LiteralPath $resolutionProfile)) { throw "Missing local resolution profile: $resolutionProfile" }
$overridePath = Join-Path (Split-Path $GamePath -Parent) 'refuge/content/paks/pakchunk99-PCVR_P.pak'
if ($ValidateOnly) {
    [pscustomobject]@{ GamePath=$GamePath; Controller=$Controller; MirrorFps=$MirrorFps; EyeWidth=$EyeWidth; Executable=$exe }
    return
}
if (Get-Process shadps4 -ErrorAction SilentlyContinue) { throw 'Close the running emulator before starting Farpoint.' }
Copy-Item -LiteralPath $resolutionProfile -Destination $overridePath -Force
$configPath = Join-Path $PSScriptRoot 'user/config.json'
if (-not (Test-Path $configPath)) { throw 'Missing emulator profile. Use the complete development installation.' }
$config = Get-Content $configPath -Raw | ConvertFrom-Json
# Supersampled stereo targets exceed the original console direct-memory budget.
# Keep a larger existing allowance if the user has configured one.
$requiredExtraMemoryMb = if ($EyeWidth -ge 3072) { 6144 } elseif ($EyeWidth -ge 2160) { 4096 } elseif ($EyeWidth -ge 1536) { 2048 } else { 0 }
$config.General.extra_dmem_in_mbytes = [Math]::Max([int]$config.General.extra_dmem_in_mbytes, $requiredExtraMemoryMb)
$config.Input.use_special_pad = $Controller -eq 'Aim'
$config.Input.special_pad_class = 9
$config.General.connected_to_network = $false
$config.General.shad_net_enabled = $false
$config | ConvertTo-Json -Depth 30 | Set-Content -LiteralPath $configPath -Encoding UTF8
@{ GamePath=$GamePath; Controller=$Controller; MirrorFps=$MirrorFps; EyeWidth=$EyeWidth } | ConvertTo-Json | Set-Content -LiteralPath $settingsFile -Encoding UTF8
$env:SHADPS4_OPENXR = '1'; $env:SHADPS4_VR = '1'
$env:SHADPS4_XR_WAIT = '0'; $env:SHADPS4_XR_PAUSE = '0'
$env:SHADPS4_XR_GAMEPAD_LAYOUT = '1'
$env:SHADPS4_XR_PAD_HAND = 'right'
$env:SHADPS4_VR_WINDOW_FPS = [string]$MirrorFps
$env:SHADPS4_FARPOINT_OFFLINE_SCORE = '1'
$env:SHADPS4_FARPOINT_EYE_WIDTH = ''
# Astro-specific patches and automation must not leak into this title.
$env:SHADPS4_TITLE_RESOLUTION = 'title'; $env:SHADPS4_TITLE_EYE_WIDTH = ''
$env:SHADPS4_VR_PACE = '1'; $env:SHADPS4_VR_FPS_CAP = ''
$env:SHADPS4_INPUT_SCRIPT = ''; $env:SHADPS4_LIVE_INPUT = ''
$env:SHADPS4_XR_TEST_PRESS = ''; $env:SHADPS4_XR_TEST_TURN = ''
$process = Start-Process -FilePath $exe -WorkingDirectory $PSScriptRoot -ArgumentList @('-g',('"' + $GamePath + '"')) -PassThru
Write-Host "Farpoint started. Log: $PSScriptRoot\user\log\shad_log.txt"
$process.WaitForExit()
if ($process.ExitCode -ne 0) { throw "Farpoint ended with code $($process.ExitCode). See user\log\shad_log.txt." }
