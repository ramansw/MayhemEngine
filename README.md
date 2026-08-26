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

Status: v0.1 core (chain recorder + registry + console demo) — see the
roadmap in the design report. No ImGui overlay yet.

## Building

```
mkdir build && cd build
cmake ..
cmake --build .
./modules/MayhemDebugger/samples/enemy_demo/enemy_demo
```

## Repository layout

```
MayhemEngine/
  modules/
    MayhemDebugger/
      include/mdbg/       public headers
      src/                implementation
      samples/enemy_demo/ standalone reproduction of the design doc's example
  docs/                   design report + API spec
```

## License

Not yet decided — add a LICENSE file before treating this as reusable by
anyone other than the author.
