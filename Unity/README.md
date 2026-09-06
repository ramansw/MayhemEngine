# MayhemDebugger (Unity Package)

## Install

1. In Unity: **Window > Package Manager > "+" > Add package from disk...**
2. Select `package.json` from this folder.

That's it. On first open Unity automatically:
- Creates `Assets/MayhemDebuggerGlobalUsings.cs` — gives every script bare access to `Check()`, `BeginChain()`, `CheckOnce()`, `SetBreakOnFail()` with no `using` line needed anywhere.
- Creates `Assets/csc.rsp` — enables C# 10 so the global using compiles.

## Usage

No setup in scripts. Just call the API directly:

```csharp
using (BeginChain("CanAttack"))
{
    if (!Check("TargetAcquired", target != null)) return;
    if (!Check("InAttackRange", distance <= attackRange,
            ("distance", distance), ("range", attackRange))) return;
}
```

```csharp
if (!CheckOnce("IsAlive", currentHealth > 0)) return;
```

```csharp
void Awake() => SetBreakOnFail("CanAttack", "InAttackRange");
```

Open **Window > MayhemDebugger** while in Play Mode to see live decision chains.

## Updating

Package Manager reads this folder directly — pull the latest repo changes and Unity picks them up on the next recompile. No reinstall needed.
