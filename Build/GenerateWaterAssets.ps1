[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$ProjectFile = Join-Path $ProjectRoot "MinecraftBedrockRealistic.uproject"
$ScriptFile = Join-Path $ProjectRoot "Scripts\create_water_material.py"

if ([string]::IsNullOrWhiteSpace($env:UE_ROOT)) {
    throw "UE_ROOT is not set. Set it to the root of an authorized Unreal Engine 5.8 installation."
}
$EditorCmd = Join-Path $env:UE_ROOT "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (!(Test-Path $EditorCmd)) {
    throw "UnrealEditor-Cmd.exe not found at '$EditorCmd'."
}
if (!(Test-Path $ScriptFile)) {
    throw "Water material generation script not found: $ScriptFile"
}

Write-Host "Importing generated wave normal maps and creating /Game/Water/Materials/M_Water_Realistic"
& $EditorCmd $ProjectFile -run=pythonscript "-script=$ScriptFile" -unattended -nop4 -nosplash -NoSound
if ($LASTEXITCODE -ne 0) {
    throw "Unreal Python water-asset generation failed with exit code $LASTEXITCODE. Inspect the Editor-Cmd log."
}
Write-Host "Water assets generated successfully."
