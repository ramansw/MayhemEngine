#include "mdbg/imgui_overlay.h"
#include "mdbg/registry.h"

#include "imgui.h"

#include <cstdio>
#include <cstddef>

namespace mdbg {

namespace {

const char* FormatValue(const NamedValue& v, char* buf, size_t bufSize) {
    switch (v.type) {
        case ValueType::Bool:   std::snprintf(buf, bufSize, "%s", v.b ? "true" : "false"); break;
        case ValueType::Int:    std::snprintf(buf, bufSize, "%d", v.i); break;
        case ValueType::Float:  std::snprintf(buf, bufSize, "%.2f", v.f); break;
        case ValueType::String: std::snprintf(buf, bufSize, "%s", v.str ? v.str : ""); break;
    }
    return buf;
}

void DrawChain(const Chain& chain) {
    const ImVec4 kPass(0.35f, 0.80f, 0.40f, 1.0f);
    const ImVec4 kFail(0.90f, 0.30f, 0.30f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Text, chain.failed ? kFail : kPass);
    bool open = ImGui::TreeNode(chain.key);
    ImGui::PopStyleColor();

    if (!open) {
        return;
    }

    for (uint8_t i = 0; i < chain.stepCount; ++i) {
        const ChainStep& step = chain.steps[i];
        ImGui::TextColored(step.passed ? kPass : kFail, "%s  [%s]", step.name, step.passed ? "PASS" : "FAIL");

        if (step.valueCount > 0 && ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            for (uint8_t v = 0; v < step.valueCount; ++v) {
                char buf[32];
                ImGui::Text("%s = %s", step.values[v].name, FormatValue(step.values[v], buf, sizeof(buf)));
            }
            ImGui::EndTooltip();
        }

        if (!step.passed) {
            ImGui::TextDisabled("  -- chain stopped here, remaining steps not reached --");
            break;
        }
    }

    ImGui::TreePop();
}

} // namespace

void DrawOverlay() {
    if (!ImGui::Begin("MayhemDebugger")) {
        ImGui::End();
        return;
    }

    Registry::Get().ForEachKey([](const std::string& key) {
        const Chain* chain = Registry::Get().Latest(key.c_str());
        if (chain) {
            DrawChain(*chain);
        }
    });

    ImGui::End();
}

} // namespace mdbg
