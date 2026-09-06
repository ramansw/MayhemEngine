#include "SNetTracePanel.h"
#include "ntr/event_log.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"

namespace {
    const FLinearColor SendColor   { 0.3f, 0.7f, 1.0f, 1.f };
    const FLinearColor RecvColor   { 0.3f, 1.0f, 0.5f, 1.f };
    const FLinearColor HeaderColor { 0.9f, 0.9f, 0.9f, 1.f };
    const FLinearColor ValueColor  { 0.9f, 0.75f, 0.3f, 1.f };
    const FLinearColor DimColor    { 0.4f, 0.4f, 0.4f, 1.f };
}

void SNetTracePanel::Construct(const FArguments& InArgs)
{
    ChildSlot [ SAssignNew(ScrollBox, SScrollBox) ];
    RebuildContent();
    RegisterActiveTimer(0.2f,
        FWidgetActiveTimerDelegate::CreateSP(this, &SNetTracePanel::OnRefreshTimer));
}

EActiveTimerReturnType SNetTracePanel::OnRefreshTimer(double, float)
{
    size_t Current = ntr::GetEventCount();
    if (Current != LastEventCount)
    {
        LastEventCount = Current;
        RebuildContent();
    }
    return EActiveTimerReturnType::Continue;
}

void SNetTracePanel::RebuildContent()
{
    ScrollBox->ClearChildren();

    FString Summary = FString::Printf(
        TEXT("Sent: %llu events / %llu bytes     Received: %llu events / %llu bytes"),
        (unsigned long long)ntr::GetTotalEventsSent(),
        (unsigned long long)ntr::GetTotalBytesSent(),
        (unsigned long long)ntr::GetTotalEventsReceived(),
        (unsigned long long)ntr::GetTotalBytesReceived());

    ScrollBox->AddSlot().Padding(6.f, 8.f, 6.f, 4.f)
    [
        SNew(STextBlock)
        .Text(FText::FromString(Summary))
        .ColorAndOpacity(FSlateColor(HeaderColor))
        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
    ];

    ScrollBox->AddSlot().Padding(4.f, 2.f)
    [
        SNew(SBox).HeightOverride(1.f)
        [ SNew(SBorder).BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.f)) ]
    ];

    size_t Count = ntr::GetEventCount();
    if (Count == 0)
    {
        ScrollBox->AddSlot().Padding(12.f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(
                TEXT("No events recorded yet.\n")
                TEXT("Use NET_TRACE_SEND(\"Name\").Bytes(n).Value(\"k\",v).Record() from any thread.")))
            .ColorAndOpacity(FSlateColor(DimColor))
            .AutoWrapText(true)
        ];
        return;
    }

    size_t Start = (Count > 64) ? Count - 64 : 0;
    for (size_t i = Count; i-- > Start; )
    {
        ntr::NetworkEvent Evt = ntr::GetEventAt(i);
        bool bSend = (Evt.direction == ntr::Direction::Send);

        FString Row = FString::Printf(TEXT("%s   %-24s   %s"),
            bSend ? TEXT("↑ SEND") : TEXT("↓ RECV"),
            UTF8_TO_TCHAR(Evt.name),
            Evt.sizeBytes > 0 ? *FString::Printf(TEXT("%u B"), Evt.sizeBytes) : TEXT("? B"));

        ScrollBox->AddSlot().Padding(8.f, 2.f, 8.f, 0.f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Row))
            .ColorAndOpacity(FSlateColor(bSend ? SendColor : RecvColor))
            .Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
        ];

        for (int v = 0; v < Evt.valueCount; ++v)
        {
            ScrollBox->AddSlot().Padding(16.f, 0.f, 8.f, 0.f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(FString::Printf(TEXT("       %s = %s"),
                    UTF8_TO_TCHAR(Evt.values[v].name),
                    UTF8_TO_TCHAR(Evt.values[v].formatted))))
                .ColorAndOpacity(FSlateColor(ValueColor))
                .Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
            ];
        }
    }
}
