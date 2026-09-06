#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SScrollBox.h"

class SMayhemDebuggerPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMayhemDebuggerPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    EActiveTimerReturnType OnRefreshTimer(double InCurrentTime, float InDeltaTime);
    void RebuildContent();
    bool HasDataChanged();

    TSharedPtr<SScrollBox> ScrollBox;

    // Tracks the last sequence number seen per chain key.
    // Rebuild only happens when any sequence changes.
    TMap<FString, uint64> LastSequences;
};
