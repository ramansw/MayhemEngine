using UnrealBuildTool;

public class MayhemDebuggerEditor : ModuleRules
{
    public MayhemDebuggerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UnrealEd",
            "ToolMenus",
            "WorkspaceMenuStructure",
            "MayhemDebugger",
        });
    }
}
