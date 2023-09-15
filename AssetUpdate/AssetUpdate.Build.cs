using UnrealBuildTool;

public class AssetUpdate : ModuleRules
{
    public AssetUpdate(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Json",
                //"SimpleOSS",  //上传用（Editor）
                "HTTP"  // 下载用 (Runtime)
            }
        );
    }
}