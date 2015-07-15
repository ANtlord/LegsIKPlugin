using UnrealBuildTool;
using System.IO;

public class LegsIKPlugin : ModuleRules
{
    public LegsIKPlugin(TargetInfo Target)
    {
        PublicIncludePaths.AddRange( new string[] { } );
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore", "UnrealEd", "BlueprintGraph", "AnimGraph" 
        });
        PrivateIncludePaths.AddRange(new string[] { "LegsIKPlugin/Private" });
    }
}