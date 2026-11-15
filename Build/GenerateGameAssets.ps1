[CmdletBinding()]
param()
$ErrorActionPreference = "Stop"
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$ProjectFile = Join-Path $ProjectRoot "MinecraftBedrockRealistic.uproject"
$EditorCmd = Join-Path $env:UE_ROOT "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if ([string]::IsNullOrWhiteSpace($env:UE_ROOT) -or !(Test-Path $EditorCmd)) { throw "Set UE_ROOT to a valid Unreal Engine 5.8 installation." }
$ContentScript = Join-Path $ProjectRoot "Scripts\create_content_assets.py"
Write-Host "Importing original PBR textures, models and audio into Unreal..."
& $EditorCmd $ProjectFile -run=pythonscript "-script=$ContentScript" -unattended -nop4 -nosplash -NoSound
if ($LASTEXITCODE -ne 0) { throw "Original game asset import failed. Inspect UnrealEditor-Cmd log." }
Write-Host "Generating water material..."
& (Join-Path $PSScriptRoot "GenerateWaterAssets.ps1")
if ($LASTEXITCODE -ne 0) { throw "Water asset generation failed." }
