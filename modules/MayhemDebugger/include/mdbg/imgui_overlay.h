#pragma once
// MayhemDebugger — optional ImGui overlay (Milestone M2).
// Deliberately its own header/translation unit: the core mdbg library
// (chain.h/registry.h) has zero dependency on ImGui, matching the
// architecture in the design report ("core does not depend on ImGui").

namespace mdbg {

// Renders the MayhemDebugger panel: one collapsible section per recorded
// chain key (green if it fully passed, red if it failed), steps listed in
// evaluation order and colored PASS/FAIL, drawing stops at the first
// failing step (later steps weren't reached, so they aren't shown as if
// they ran). Hover a step to see its named values.
//
// Call once per frame, between ImGui::NewFrame() and ImGui::Render().
void DrawOverlay();

} // namespace mdbg
