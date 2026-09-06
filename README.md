# MayhemEngine

Game debugging toolset with two components that work together in both **Unreal Engine** and **Unity**.

| Tool | What it answers |
|---|---|
| **MayhemDebugger** | *Why did this decision happen?* — records every condition checked and shows exactly which one failed |
| **NetTrace** | *What actually went over the wire?* — rolling log of network send/receive events with sizes and tagged values |

Both are included in the same plugin/package. Zero external dependencies. No heap allocation in hot paths.

---

## Unreal Engine — Installation

**Supports UE 5.3+**

1. Copy the `UE/MayhemDebugger/` folder into your project's `Plugins/` directory:
   ```
   YourProject/
   └── Plugins/
       └── MayhemDebugger/      ← drop it here
           ├── MayhemDebugger.uplugin
           └── Source/
   ```

2. Right-click your `.uproject` → **Generate Visual Studio project files**

3. Open your project's `.uproject` in a text editor and add to the `"Plugins"` array:
   ```json
   { "Name": "MayhemDebugger", "Enabled": true }
   ```

4. Add `"MayhemDebugger"` to your module's `PublicDependencyModuleNames` in `YourModule.Build.cs`:
   ```csharp
   PublicDependencyModuleNames.AddRange(new string[] { "Core", "MayhemDebugger" });
   ```

5. Build and run. Two panels appear under **Tools → Debug**:
   - **MayhemDebugger** — live decision-chain viewer
   - **NetTrace** — live network event viewer

### UE Usage

```cpp
#include "MayhemDebugger.h"
#include "MayhemDebuggerNetTrace.h"

// Decision chain — records why a decision happened
void AMyCharacter::DoJump()
{
    DEBUG_CHAIN("DoJump");
    if (!DEBUG_CHECK("IsAlive",    currentHealth > 0.f, mdbg::V("health", currentHealth))) return;
    if (!DEBUG_CHECK("HasControl", GetController() != nullptr)) return;
    Jump();
}

// Network tracing — records what went over the wire
void AMyCharacter::OnRep_Health()
{
    NET_TRACE_RECEIVE("Rep_Health")
        .Bytes(sizeof(float))
        .Value("health", currentHealth)
        .Record();
}
```

---

## Unity — Installation

**Supports Unity 2020.3+**

### Option A — UPM Git URL (recommended, no manual steps)

1. Open **Window → Package Manager**
2. Click **+** → **Add package from git URL**
3. Enter:
   ```
   https://github.com/YOUR_USERNAME/MayhemEngine.git?path=/Unity
   ```
   To pin to a specific version:
   ```
   https://github.com/YOUR_USERNAME/MayhemEngine.git?path=/Unity#v1.0.0
   ```

### Option B — Download zip

Download `MayhemDebugger-Unity-vX.X.X.zip` from [Releases](../../releases), extract it, and import via **Assets → Import Package → Custom Package**.

### Unity Usage

```csharp
using MayhemDebugger;
using MayhemDebugger.NetTrace;

// Decision chain
void Attack()
{
    using var chain = MDBG.Chain("Attack");
    if (!chain.Check("TargetInRange",  distanceToTarget < attackRange)) return;
    if (!chain.Check("CooldownReady",  cooldownTimer <= 0f))            return;
    DealDamage();
}

// Network event
void OnPacketSent(int bytes)
{
    NTR.RecordSend("PlayerPosition").Bytes(bytes).Value("pos", transform.position.ToString()).Record();
}
```

Open **Window → MayhemDebugger → Debugger** and **Window → MayhemDebugger → NetTrace** for the live viewers.

---

## Building from Source

The `modules/` folder contains the full C++ source, tests, and demos.

**Requirements:** CMake 3.16+, C++17 compiler (MSVC, GCC, Clang)

```bash
cmake -S modules/NetTraceUnityBridge -B build/ntr
cmake --build build/ntr --config Release
```

The built `NetTraceUnityBridge.dll` goes into `Unity/Runtime/Plugins/x86_64/`.

---

## License

MIT
