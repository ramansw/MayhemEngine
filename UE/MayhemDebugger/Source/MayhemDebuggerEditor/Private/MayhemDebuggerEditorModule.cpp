#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Framework/Docking/TabManager.h"
#include "Styling/AppStyle.h"
#include "SMayhemDebuggerPanel.h"
#include "SNetTracePanel.h"

static const FName MayhemDebuggerTabName("MayhemDebugger");
static const FName NetTraceTabName("MayhemDebugger_NetTrace");

class FMayhemDebuggerEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        auto& TM         = *FGlobalTabmanager::Get();
        auto  DebugGroup = WorkspaceMenu::GetMenuStructure().GetDeveloperToolsDebugCategory();
        const FName StyleSet = FAppStyle::GetAppStyleSetName();

        TM.RegisterNomadTabSpawner(
            MayhemDebuggerTabName,
            FOnSpawnTab::CreateRaw(this, &FMayhemDebuggerEditorModule::SpawnDebuggerTab))
            .SetDisplayName(FText::FromString(TEXT("MayhemDebugger")))
            .SetTooltipText(FText::FromString(TEXT("Live decision-chain viewer")))
            .SetIcon(FSlateIcon(StyleSet, "Icons.Debug"))
            .SetGroup(DebugGroup);

        TM.RegisterNomadTabSpawner(
            NetTraceTabName,
            FOnSpawnTab::CreateRaw(this, &FMayhemDebuggerEditorModule::SpawnNetTraceTab))
            .SetDisplayName(FText::FromString(TEXT("NetTrace")))
            .SetTooltipText(FText::FromString(TEXT("Live network event viewer")))
            .SetIcon(FSlateIcon(StyleSet, "Profiler.Tab"))
            .SetGroup(DebugGroup);
    }

    virtual void ShutdownModule() override
    {
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MayhemDebuggerTabName);
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(NetTraceTabName);
    }

private:
    TSharedRef<SDockTab> SpawnDebuggerTab(const FSpawnTabArgs&)
    {
        return SNew(SDockTab).TabRole(ETabRole::NomadTab) [ SNew(SMayhemDebuggerPanel) ];
    }

    TSharedRef<SDockTab> SpawnNetTraceTab(const FSpawnTabArgs&)
    {
        return SNew(SDockTab).TabRole(ETabRole::NomadTab) [ SNew(SNetTracePanel) ];
    }
};

IMPLEMENT_MODULE(FMayhemDebuggerEditorModule, MayhemDebuggerEditor)
