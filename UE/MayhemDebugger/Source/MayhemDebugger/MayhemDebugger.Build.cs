using UnrealBuildTool;

public class MayhemDebugger : ModuleRules
{
    public MayhemDebugger(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Allow standard C++ library headers (<cstdint>, <cstring>, <atomic>, <mutex>, <chrono>)
        // used by the plugin's native source without UE warnings treating them as errors.
        CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;

        PublicDependencyModuleNames.AddRange(new string[] { "Core" });
    }
}
