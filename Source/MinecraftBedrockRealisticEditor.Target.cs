using UnrealBuildTool;
using System.Collections.Generic;

public class MinecraftBedrockRealisticEditorTarget : TargetRules
{
    public MinecraftBedrockRealisticEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("MinecraftBedrockRealistic");
    }
}
