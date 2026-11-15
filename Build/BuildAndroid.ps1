[CmdletBinding()]
param(
    [string]$OutputDirectory = ""
)

$ErrorActionPreference = "Stop"
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$ProjectFile = Join-Path $ProjectRoot "MinecraftBedrockRealistic.uproject"

if ([string]::IsNullOrWhiteSpace($env:UE_ROOT)) {
    throw "UE_ROOT is not set. Set it to the root of an authorized Unreal Engine 5.8 installation on the self-hosted runner."
}

$UAT = Join-Path $env:UE_ROOT "Engine\Build\BatchFiles\RunUAT.bat"
if (!(Test-Path $UAT)) {
    throw "RunUAT.bat not found at '$UAT'. UE_ROOT must point to the Unreal Engine installation root."
}
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = Join-Path $ProjectRoot "Build\AndroidPackage"
}
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null

# Import source normal maps and create the UE Material asset before cooking.
& (Join-Path $PSScriptRoot "GenerateGameAssets.ps1")

Write-Host "Packaging $ProjectFile for Android (ARM64) using $env:UE_ROOT"
& $UAT BuildCookRun `
    "-project=$ProjectFile" `
    -noP4 `
    -platform=Android `
    -clientconfig=Development `
    -build `
    -cook `
    -stage `
    -pak `
    -package `
    -archive `
    "-archivedirectory=$OutputDirectory" `
    -utf8output

if ($LASTEXITCODE -ne 0) {
    throw "Unreal Automation Tool failed with exit code $LASTEXITCODE"
}

$Packages = Get-ChildItem -Path $OutputDirectory -Recurse -Include *.apk,*.aab -ErrorAction SilentlyContinue
if (!$Packages) {
    throw "UAT exited successfully but no APK/AAB was found under '$OutputDirectory'. Inspect the UAT log and Android platform settings."
}
Write-Host "Package output:"
$Packages | ForEach-Object { Write-Host $_.FullName }
