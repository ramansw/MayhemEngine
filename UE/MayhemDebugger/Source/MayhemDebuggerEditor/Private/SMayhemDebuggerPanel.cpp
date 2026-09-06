#include "SMayhemDebuggerPanel.h"
#include "mdbg/registry.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"

namespace {
    const FLinearColor PassColor  { 0.2f, 0.8f, 0.2f, 1.f };
    const FLinearColor FailColor  { 0.9f, 0.2f, 0.2f, 1.f };
    const FLinearColor DimColor   { 0.4f, 0.4f, 0.4f, 1.f };
    const FLinearColor ValueColor { 0.9f, 0.75f, 0.3f, 1.f };
}

void SMayhemDebuggerPanel::Construct(const FArguments& InArgs)
{
    ChildSlot [ SAssignNew(ScrollBox, SScrollBox) ];
    RebuildContent();
    RegisterActiveTimer(0.2f,
        FWidgetActiveTimerDelegate::CreateSP(this, &SMayhemDebuggerPanel::OnRefreshTimer));
}

EActiveTimerReturnType SMayhemDebuggerPanel::OnRefreshTimer(double, float)
{
    if (HasDataChanged())
        RebuildContent();
    return EActiveTimerReturnType::Continue;
}

bool SMayhemDebuggerPanel::HasDataChanged()
{
    bool bChanged = false;
    TMap<FString, uint64> Current;

    mdbg::Registry::Get().ForEachKey([&](const std::string& Key)
    {
        const mdbg::Chain* Chain = mdbg::Registry::Get().Latest(Key.c_str());
        if (!Chain) return;

        FString UKey = UTF8_TO_TCHAR(Key.c_str());
        Current.Add(UKey, Chain->sequence);

        const uint64* Last = LastSequences.Find(UKey);
        if (!Last || *Last != Chain->sequence)
            bChanged = true;
    });

    // A key was removed
    if (Current.Num() != LastSequences.Num())
        bChanged = true;

    if (bChanged)
        LastSequences = MoveTemp(Current);

    return bChanged;
}

void SMayhemDebuggerPanel::RebuildContent()
{
    ScrollBox->ClearChildren();

    bool bAnyChain = false;

    mdbg::Registry::Get().ForEachKey([&](const std::string& Key)
    {
        const mdbg::Chain* Chain = mdbg::Registry::Get().Latest(Key.c_str());
        if (!Chain) return;

        bAnyChain = true;
        const bool bFailed = Chain->failed;

        FString HeaderText = FString::Printf(TEXT("%s   [%s]"),
            UTF8_TO_TCHAR(Chain->key),
            bFailed ? TEXT("FAILED") : TEXT("OK"));

        ScrollBox->AddSlot().Padding(4.f, 6.f, 4.f, 2.f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(HeaderText))
            .ColorAndOpacity(FSlateColor(bFailed ? FailColor : PassColor))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
        ];

        for (uint8_t s = 0; s < Chain->stepCount; ++s)
        {
            const mdbg::ChainStep& Step = Chain->steps[s];

            FString StepText = FString::Printf(TEXT("    %s   [%s]"),
                UTF8_TO_TCHAR(Step.name),
                Step.passed ? TEXT("PASS") : TEXT("FAIL"));

            ScrollBox->AddSlot().Padding(16.f, 1.f, 4.f, 1.f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(StepText))
                .ColorAndOpacity(FSlateColor(Step.passed ? PassColor : FailColor))
                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
            ];

            for (uint8_t v = 0; v < Step.valueCount; ++v)
            {
                const mdbg::NamedValue& Val = Step.values[v];
                FString Formatted;
                switch (Val.type)
                {
                    case mdbg::ValueType::Bool:   Formatted = Val.b ? TEXT("true") : TEXT("false"); break;
                    case mdbg::ValueType::Int:    Formatted = FString::FromInt(Val.i);              break;
                    case mdbg::ValueType::Float:  Formatted = FString::Printf(TEXT("%.2f"), Val.f); break;
                    case mdbg::ValueType::String: Formatted = UTF8_TO_TCHAR(Val.str);               break;
                }

                ScrollBox->AddSlot().Padding(24.f, 0.f, 4.f, 0.f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(FString::Printf(TEXT("        %s = %s"),
                        UTF8_TO_TCHAR(Val.name), *Formatted)))
                    .ColorAndOpacity(FSlateColor(ValueColor))
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
                ];
            }

            if (!Step.passed) break;
        }

        ScrollBox->AddSlot().Padding(4.f, 4.f)
        [
            SNew(SBox).HeightOverride(1.f)
            [ SNew(SBorder).BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.f)) ]
        ];
    });

    if (!bAnyChain)
    {
        ScrollBox->AddSlot().Padding(12.f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(
                TEXT("No chains recorded yet.\n")
                TEXT("Wrap logic in DEBUG_CHAIN(\"Key\") / DEBUG_CHECK(\"Step\", cond).")))
            .ColorAndOpacity(FSlateColor(DimColor))
            .AutoWrapText(true)
        ];
    }
}
