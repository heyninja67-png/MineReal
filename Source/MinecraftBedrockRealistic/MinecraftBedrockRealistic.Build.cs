using UnrealBuildTool;

public class MinecraftBedrockRealistic : ModuleRules
{
    public MinecraftBedrockRealistic(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore",
            "ProceduralMeshComponent", "UMG", "Slate", "SlateCore",
            "AIModule", "NavigationSystem"
        });
    }
}
