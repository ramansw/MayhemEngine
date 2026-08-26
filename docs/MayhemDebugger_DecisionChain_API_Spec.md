# MayhemDebugger — Decision Chain API & Implementation Spec

Companion to `MayhemDebugger_Design_Report.docx`. This is the implementation-level sketch: data structures, macro expansion, storage, and the ImGui rendering pass. Personal project / portfolio scope — kept intentionally small (see Section 7, "Scope," in the design report).

---

## 1. Goals for the API surface

- Wrapping an existing `if` in `DEBUG_CHECK` must cost almost nothing to type or read.
- The macro never changes program behavior — it evaluates the real condition and returns its real result. Disabling the tool must not change control flow.
- No heap allocation in the hot path when possible; a fixed-capacity ring buffer per chain key is enough for v0.1.
- Single calling thread per chain (see Section 9.5 of the design report) — no locks in v0.1.

---

## 2. Core data structures

```cpp
namespace mdbg {

enum class ValueType : uint8_t { Bool, Int, Float, String };

struct NamedValue {
    const char* name;
    ValueType   type;
    union {
        bool  b;
        int   i;
        float f;
    };
    const char* str; // used when type == String; points into a small interned buffer
};

struct ChainStep {
    const char* name;        // "InAttackRange"
    bool        passed;      // result of the wrapped condition
    NamedValue  values[4];   // fixed cap — enough for "distance <= range" style checks
    uint8_t     valueCount;
    const char* file;
    int         line;
};

struct Chain {
    const char* key;             // "CanAttack" — stable pointer, expected to be a string literal
    ChainStep   steps[16];       // fixed cap per chain; overflow steps are dropped, not UB
    uint8_t     stepCount;
    bool        failed;          // true once any step fails
    uint64_t    frameNumber;     // set by the recorder when the chain closes
};

} // namespace mdbg
```

Fixed-capacity arrays are deliberate: no allocator interaction, predictable memory (`sizeof(Chain)` is small and constant), and a step/value count past the cap is a silent truncation rather than a crash — acceptable for a personal debugging tool, not acceptable if this ever became a shipped product.

---

## 3. Macros and RAII scope

```cpp
#if MDBG_ENABLED

struct ChainScope {
    ChainScope(const char* key);   // pushes a new Chain onto the thread-local stack
    ~ChainScope();                 // pops it, hands the finished Chain to the registry

    // returns false immediately if condition == false, after recording the step
    bool Check(const char* name, bool condition, std::initializer_list<NamedValue> values,
               const char* file, int line);
};

#define DEBUG_CHAIN(key) ::mdbg::ChainScope _mdbg_scope_##__LINE__(key)

#define DEBUG_CHECK(name, cond, ...) \
    ::mdbg::CurrentChainScope()->Check(name, (cond), {__VA_ARGS__}, __FILE__, __LINE__)

#else // MDBG_ENABLED == 0

#define DEBUG_CHAIN(key)            ((void)0)
#define DEBUG_CHECK(name, cond, ...) (cond)

#endif
```

`CurrentChainScope()` reads a thread-local pointer to whichever `ChainScope` is innermost on the current thread's stack. If `DEBUG_CHECK` is called with no open `DEBUG_CHAIN`, it should be a no-op that still returns `cond` — instrumenting a stray condition without a chain shouldn't crash, it should just not record anything.

Note on `NamedValue` construction: `"distance", distance, "range", attackRange` in the example needs a small variadic-to-`NamedValue` conversion. The cleanest v0.1 approach is a helper macro that pairs adjacent arguments, or simply requiring callers to pass `mdbg::V("distance", distance)` pairs explicitly:

```cpp
DEBUG_CHECK("InAttackRange", distance <= attackRange,
            mdbg::V("distance", distance), mdbg::V("range", attackRange));
```

This is slightly more verbose than the design report's shorthand but avoids fragile variadic-pairing macro tricks — worth the trade for a solo project where debugging the macro itself is time you don't get back.

---

## 4. Registry (ring buffer per key)

```cpp
class Registry {
public:
    static Registry& Get();

    void Publish(const Chain& finished);          // called by ChainScope's destructor
    const Chain* Latest(const char* key) const;    // for the ImGui panel
    void ForEachKey(std::function<void(const char*)> fn) const; // populate category list

private:
    struct Slot { std::string key; Chain chain; };
    std::vector<Slot> slots;   // linear scan is fine — expect tens of chain keys, not thousands
};
```

v0.1 only needs `Latest()` — one chain per key, overwritten every time that chain closes. This satisfies the whole design report (Sections 2, 7, 8): open the overlay, see the most recent evaluation of `CanAttack`. Cross-frame history (multiple past chains per key, scrub-back) is explicitly the "Stretch" roadmap item, not v0.1 — don't build the ring-buffer-of-N version until the single-latest version is actually working and useful.

---

## 5. ImGui frontend (pseudocode)

```cpp
void DrawMayhemDebuggerOverlay() {
    if (!ImGui::Begin("MayhemDebugger")) { ImGui::End(); return; }

    Registry::Get().ForEachKey([](const char* key) {
        if (ImGui::TreeNode(key)) {
            const Chain* chain = Registry::Get().Latest(key);
            for (uint8_t i = 0; i < chain->stepCount; i++) {
                const ChainStep& step = chain->steps[i];
                ImVec4 color = step.passed ? GREEN : RED;
                ImGui::TextColored(color, "%s  [%s]", step.name, step.passed ? "PASS" : "FAIL");
                if (ImGui::IsItemHovered()) {
                    for (uint8_t v = 0; v < step.valueCount; v++)
                        ImGui::Text("  %s = %s", step.values[v].name, FormatValue(step.values[v]));
                }
                if (!step.passed) break; // steps after the first failure weren't reached — don't draw them as if they ran
            }
            ImGui::TreePop();
        }
    });

    ImGui::End();
}
```

This directly produces the table from Section 2 of the design report: green PASS rows down to the red FAIL row, nothing drawn below it (matching "not reached" rather than showing false PASS/FAIL for steps that never executed).

---

## 6. What v0.1 explicitly does not attempt

(Mirrors Section 7/9 of the design report — repeated here so the implementation doesn't quietly grow scope while coding.)

- No parsing/decomposition of compound `&&`/`||` expressions — one `DEBUG_CHECK` per named condition.
- No cross-thread chain merging — `CurrentChainScope()` is a plain `thread_local`, not a lock-free structure.
- No persistence to disk and no multi-frame history — `Registry::Latest()` only ever holds the most recent chain per key.
- No reflection — `NamedValue` supports exactly `bool`, `int`, `float`, `string`, nothing generic.

---

## 7. Suggested file layout

```
MayhemDebugger/
  include/mdbg/chain.h        // ChainStep, Chain, NamedValue, macros
  include/mdbg/registry.h
  src/chain.cpp
  src/registry.cpp
  src/imgui_overlay.cpp       // optional, behind MDBG_WITH_IMGUI
  samples/enemy_demo/         // the standalone reproduction of the design report's example
  CMakeLists.txt
```

Keep the core (`chain.h`/`chain.cpp`/`registry.*`) free of any ImGui include — the overlay is a separate optional translation unit, consistent with Section 6 of the design report ("core does not depend on... ImGui").
