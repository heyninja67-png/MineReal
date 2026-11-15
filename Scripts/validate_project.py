#!/usr/bin/env python3
"""Offline validation only. This does not replace UnrealBuildTool, UHT, cooking or device tests."""
from pathlib import Path
import ast
import json
import struct
import sys
import wave

ROOT = Path(__file__).resolve().parents[1]
errors = []

def require(condition, message):
    if not condition:
        errors.append(message)

required = [
    "MinecraftBedrockRealistic.uproject",
    "Source/MinecraftBedrockRealistic/MinecraftBedrockRealistic.Build.cs",
    "Source/MinecraftBedrockRealistic/Public/MCRBlockTypes.h",
    "Source/MinecraftBedrockRealistic/Public/MCRBlockChunk.h",
    "Source/MinecraftBedrockRealistic/Private/MCRBlockChunk.cpp",
    "Source/MinecraftBedrockRealistic/Public/MCRWorldManager.h",
    "Source/MinecraftBedrockRealistic/Private/MCRWorldManager.cpp",
    "Source/MinecraftBedrockRealistic/Public/MCRWaterSurface.h",
    "Source/MinecraftBedrockRealistic/Private/MCRWaterSurface.cpp",
    "Source/MinecraftBedrockRealistic/Public/MCRPlayerCharacter.h",
    "Source/MinecraftBedrockRealistic/Private/MCRPlayerCharacter.cpp",
    "Source/MinecraftBedrockRealistic/Public/MCRInventoryComponent.h",
    "Source/MinecraftBedrockRealistic/Private/MCRInventoryComponent.cpp",
    "Source/MinecraftBedrockRealistic/Public/MCRCraftingComponent.h",
    "Source/MinecraftBedrockRealistic/Private/MCRCraftingComponent.cpp",
    "Source/MinecraftBedrockRealistic/Public/MCRSurvivalComponent.h",
    "Source/MinecraftBedrockRealistic/Private/MCRSurvivalComponent.cpp",
    "Source/MinecraftBedrockRealistic/Public/MCRDayNightWeather.h",
    "Source/MinecraftBedrockRealistic/Private/MCRDayNightWeather.cpp",
    "Source/MinecraftBedrockRealistic/Public/MCRCreatureActor.h",
    "Source/MinecraftBedrockRealistic/Private/MCRCreatureActor.cpp",
    "Source/MinecraftBedrockRealistic/Public/MCRInventoryWidget.h",
    "Source/MinecraftBedrockRealistic/Private/MCRInventoryWidget.cpp",
    "Source/MinecraftBedrockRealistic/Public/MCRMobileHUD.h",
    "Source/MinecraftBedrockRealistic/Private/MCRMobileHUD.cpp",
    "Source/MinecraftBedrockRealistic/Public/MCRWorldSaveGame.h",
    "Scripts/create_content_assets.py",
    "Scripts/create_water_material.py",
    "Scripts/generate_original_assets.py",
    "Build/GenerateGameAssets.ps1",
    "Build/GenerateWaterAssets.ps1",
    "Build/BuildAndroid.ps1",
    ".github/workflows/android-build.yml",
    "README.md",
]
for relative in required:
    require((ROOT / relative).is_file(), f"Missing required file: {relative}")

try:
    project = json.loads((ROOT / "MinecraftBedrockRealistic.uproject").read_text(encoding="utf-8"))
except Exception as exc:
    project = {}
    require(False, f"Invalid .uproject JSON: {exc}")

require(project.get("EngineAssociation") == "5.8", "EngineAssociation must be 5.8")
require(any(m.get("Name") == "MinecraftBedrockRealistic" for m in project.get("Modules", [])), "Runtime module is missing")
plugins = {p.get("Name"): p.get("Enabled") for p in project.get("Plugins", [])}
for name in ("ProceduralMeshComponent", "PythonScriptPlugin", "EditorScriptingUtilities"):
    require(plugins.get(name) is True, f"Required plugin is not enabled: {name}")

engine_ini = (ROOT / "Config/DefaultEngine.ini").read_text(encoding="utf-8")
for setting in ("TargetSDKVersion=35", "MinSDKVersion=26", "bBuildForArm64=True", "bSupportsVulkan=True", "PackageName=com.private.mcrrealistic"):
    require(setting in engine_ini, f"Android config is missing: {setting}")

# Verify source textures expected by the Editor importer.
material_names = ["Grass", "Dirt", "Stone", "Sand", "Wood", "Leaves", "CoalOre", "IronOre",
                  "Cobblestone", "Planks", "Gravel", "Sandstone", "Snow", "Ice", "Clay",
                  "CopperOre", "GoldOre", "DiamondOre"]
texture_dir = ROOT / "Content/Blocks/Source"
texture_files = sorted(texture_dir.glob("*.png"))
require(len(texture_files) == 54, f"Expected 54 block PBR PNGs, found {len(texture_files)}")
for name in material_names:
    for suffix in ("BC", "N", "R"):
        path = texture_dir / f"T_{name}_{suffix}.png"
        require(path.is_file(), f"Missing PBR texture: {path.relative_to(ROOT)}")
        if path.is_file():
            try:
                header = path.read_bytes()[:24]
                require(header[:8] == b"\x89PNG\r\n\x1a\n", f"Invalid PNG signature: {path.name}")
                width, height = struct.unpack(">II", header[16:24])
                require((width, height) == (512, 512), f"Unexpected PNG dimensions: {path.name} ({width}x{height})")
            except Exception as exc:
                require(False, f"Could not inspect PNG {path.name}: {exc}")

water_textures = [ROOT / "Content/Water/Source/T_WaterNormal_A.png", ROOT / "Content/Water/Source/T_WaterNormal_B.png"]
for path in water_textures:
    require(path.is_file(), f"Missing water source normal map: {path.name}")

# Validate source geometry indices.
model_files = sorted((ROOT / "Content/Models/Source").glob("*.obj"))
require(len(model_files) == 10, f"Expected 10 OBJ meshes, found {len(model_files)}")
for path in model_files:
    vertex_count = 0
    face_count = 0
    max_face_index = 0
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith("v "):
            vertex_count += 1
        elif line.startswith("f "):
            face_count += 1
            for item in line.split()[1:]:
                try:
                    index = int(item.split("/")[0])
                    max_face_index = max(max_face_index, index)
                except ValueError:
                    require(False, f"Invalid OBJ face index in {path.name}: {item}")
    require(vertex_count >= 8 and face_count >= 6, f"OBJ looks empty: {path.name}")
    require(max_face_index <= vertex_count, f"OBJ face references missing vertex: {path.name}")
    require((path.parent / (path.stem + ".mtl")).is_file(), f"OBJ material library missing: {path.stem}.mtl")

# Validate synthesized audio files.
audio_files = sorted((ROOT / "Content/Audio/Source").glob("*.wav"))
require(len(audio_files) == 8, f"Expected 8 WAV files, found {len(audio_files)}")
for path in audio_files:
    try:
        with wave.open(str(path), "rb") as wav:
            require(wav.getnchannels() == 1, f"WAV must be mono: {path.name}")
            require(wav.getsampwidth() == 2, f"WAV must be 16-bit PCM: {path.name}")
            require(wav.getframerate() == 22050, f"WAV sample rate mismatch: {path.name}")
            require(wav.getnframes() > 1000, f"WAV is too short or empty: {path.name}")
    except Exception as exc:
        require(False, f"Invalid WAV {path.name}: {exc}")

# Python syntax for scripts that are executed by the build/asset pipeline.
for relative in ("Scripts/create_content_assets.py", "Scripts/create_water_material.py", "Scripts/generate_original_assets.py"):
    try:
        ast.parse((ROOT / relative).read_text(encoding="utf-8"), filename=relative)
    except Exception as exc:
        require(False, f"Python syntax error in {relative}: {exc}")

# Core system symbols: a structural check only, not a C++ compiler.
source_text = "\n".join(p.read_text(encoding="utf-8", errors="replace") for p in (ROOT / "Source").rglob("*.cpp"))
for symbol in ("GenerateBlocks", "RebuildMesh", "BreakBlock", "PlaceBlock", "SaveWorld", "StreamChunks",
               "RebuildSurface", "EvaluateWave", "AddBlock", "MoveOrMergeSlots", "SplitHalf", "Craft(FName",
               "ApplyFallDamage", "SetCreativeMode", "SetWeather", "SpawnWildlife", "ToggleInventory"):
    require(symbol in source_text, f"Expected gameplay implementation symbol missing: {symbol}")

workflow = (ROOT / ".github/workflows/android-build.yml").read_text(encoding="utf-8")
for token in ("self-hosted", "ue-5-8", "android", "BuildAndroid.ps1", "actions/upload-artifact@v4"):
    require(token in workflow, f"GitHub workflow missing expected setup: {token}")

if errors:
    print(f"FAIL: {len(errors)} structural validation error(s)")
    for error in errors:
        print(" - " + error)
    sys.exit(1)

source_bytes = sum(p.stat().st_size for p in (ROOT / "Content").rglob("*") if p.is_file())
print("PASS: project structure, JSON, Android settings and source asset integrity verified.")
print(f"Assets: {len(texture_files)} block PBR PNGs + 2 water normals + {len(model_files)} OBJ models + {len(audio_files)} WAV files.")
print(f"Content source size: {source_bytes:,} bytes ({source_bytes/1024/1024:.2f} MiB).")
print("NOTE: this does not compile C++, run UnrealHeaderTool, import assets, cook content, create an APK or test gameplay on Android.")
