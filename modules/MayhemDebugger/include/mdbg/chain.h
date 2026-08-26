#pragma once
// MayhemDebugger Decision Chain core types
// See docs/MayhemDebugger_DecisionChain_API_Spec.md for the full design rationale.

#include <cstdint>
#include <cstddef>
#include <initializer_list>

#ifndef MDBG_ENABLED
#define MDBG_ENABLED 1
#endif

namespace mdbg {

enum class ValueType : uint8_t { Bool, Int, Float, String };

struct NamedValue {
    const char* name = nullptr;
    ValueType   type = ValueType::Bool;
    union {
        bool  b;
        int   i;
        float f;
    };
    const char* str = nullptr; // used when type == String

    NamedValue() : b(false) {}
};

inline NamedValue V(const char* name, bool value) {
    NamedValue v; v.name = name; v.type = ValueType::Bool; v.b = value; return v;
}
inline NamedValue V(const char* name, int value) {
    NamedValue v; v.name = name; v.type = ValueType::Int; v.i = value; return v;
}
inline NamedValue V(const char* name, float value) {
    NamedValue v; v.name = name; v.type = ValueType::Float; v.f = value; return v;
}
inline NamedValue V(const char* name, const char* value) {
    NamedValue v; v.name = name; v.type = ValueType::String; v.str = value; return v;
}

constexpr int kMaxValuesPerStep = 4;
constexpr int kMaxStepsPerChain = 16;

// One evaluated condition inside a chain: what was checked, what it was
// compared against, and whether it passed. Fixed-size on purpose — v0.1
// is a personal-scale tool, not a production SDK (see design report Sec. 7).
struct ChainStep {
    const char* name = nullptr;
    bool        passed = false;
    NamedValue  values[kMaxValuesPerStep];
    uint8_t     valueCount = 0;
    const char* file = nullptr;
    int         line = 0;
};

// A single evaluation of a named decision (e.g. "CanAttack"): the ordered
// steps it went through, and whether any of them failed.
struct Chain {
    const char* key = nullptr;
    ChainStep   steps[kMaxStepsPerChain];
    uint8_t     stepCount = 0;
    bool        failed = false;
    uint64_t    sequence = 0; // monotonically increasing, set when the chain closes
};

// RAII scope: opens a chain on construction, publishes it to the Registry
// and pops itself off the thread-local chain stack on destruction.
class ChainScope {
public:
    explicit ChainScope(const char* key);
    ~ChainScope();

    ChainScope(const ChainScope&) = delete;
    ChainScope& operator=(const ChainScope&) = delete;

    // Records one step. Always returns `condition` unchanged — this must
    // never alter program behavior, enabled or disabled.
    bool Check(const char* name, bool condition,
               std::initializer_list<NamedValue> values,
               const char* file, int line);

private:
    Chain chain_;
};

// Innermost open ChainScope on the calling thread, or nullptr if none is open.
// v0.1 is single-thread-per-chain by design (see API spec Sec 9.5 / Sec 1) —
// this is a plain thread_local, not a lock-free structure.
ChainScope* CurrentChainScope();

} // namespace mdbg

#if MDBG_ENABLED

#define MDBG_CONCAT_INNER(a, b) a##b
#define MDBG_CONCAT(a, b) MDBG_CONCAT_INNER(a, b)

#define DEBUG_CHAIN(key) ::mdbg::ChainScope MDBG_CONCAT(_mdbg_scope_, __LINE__)(key)

#define DEBUG_CHECK(name, cond, ...)                                          \
    (::mdbg::CurrentChainScope()                                              \
         ? ::mdbg::CurrentChainScope()->Check((name), (cond), {__VA_ARGS__},  \
                                               __FILE__, __LINE__)             \
         : (cond))

#else

#define DEBUG_CHAIN(key) ((void)0)
#define DEBUG_CHECK(name, cond, ...) (cond)

#endif
