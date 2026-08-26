# MayhemEngine

A small, personal framework of game-dev tooling. This is a from-scratch
learning/portfolio project, not a production engine — the goal is a handful
of focused, engine-agnostic C++ modules, each solving one real problem well,
rather than a monolithic engine.

## Modules

### MayhemDebugger

A decision-chain debugger: instead of showing the *current state* of a
gameplay/AI decision (what most engine debuggers and profilers already do),
it records the *chain of checks* that produced a decision and shows exactly
which one failed, and why.

Example: an enemy isn't attacking. Instead of a state dump —

```
AI State: Combat
Target: Player
Distance: 7.4m
```

— MayhemDebugger records:

```
== CanAttack ==
  TargetAcquired   [PASS]
  InAttackRange    [FAIL]  distance=5.70  range=5.00
  -- chain stopped here, remaining steps not reached --
```

See `docs/MayhemDebugger_Design_Report.docx` for the full design rationale
and competitive research (why this isn't just a smaller Tracy/Palanteer/
Unreal Gameplay Debugger), and
`docs/MayhemDebugger_DecisionChain_API_Spec.md` for the implementation-level
API/data-structure spec.

Status:
- Milestone M1 (core chain recorder + registry + console demo) — done.
- Milestone M2 (ImGui overlay + live windowed demo) — implemented, backend
  is GLFW + OpenGL3. **Not yet build-verified** — first configure needs
  internet access to fetch Dear ImGui and GLFW via CMake FetchContent.
- M3 (portfolio demo recording) and the stretch goals (capture/replay,
  spatial draw tie-in, compound-expression support) — not started.

## Building

```
mkdir build && cd build
cmake ..
cmake --build .
```

Two demos get built:

- `modules/MayhemDebugger/samples/enemy_demo/enemy_demo` — console-only,
  prints the decision chain once and exits.
- `modules/MayhemDebugger/samples/overlay_demo/overlay_demo` — a live window
  (GLFW + OpenGL3 + ImGui) with sliders to move the player and toggle line
  of sight/cooldown, so you can watch the chain in `mdbg::DrawOverlay()`
  update in real time.

Pass `-DMDBG_WITH_IMGUI=OFF` to `cmake` to skip the overlay/GLFW/ImGui
entirely and build only the console demo (no internet needed in that case).

## Repository layout

```
MayhemEngine/
  modules/
    MayhemDebugger/
      include/mdbg/           public headers (chain.h, registry.h, imgui_overlay.h)
      src/                    implementation
      samples/enemy_demo/     console-only reproduction of the design doc's example
      samples/overlay_demo/   live ImGui overlay, same scenario, interactive
  docs/                       design report + API spec
```

## License

Not yet decided — add a LICENSE file before treating this as reusable by
anyone other than the author.
