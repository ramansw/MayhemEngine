#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMayhemDebuggerModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FMayhemDebuggerModule, MayhemDebugger)
