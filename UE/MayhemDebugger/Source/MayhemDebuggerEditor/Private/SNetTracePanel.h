#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SScrollBox.h"

class SNetTracePanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SNetTracePanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    EActiveTimerReturnType OnRefreshTimer(double InCurrentTime, float InDeltaTime);
    void RebuildContent();

    TSharedPtr<SScrollBox> ScrollBox;

    // Rebuild only when new events arrive.
    size_t LastEventCount = SIZE_MAX;
};
