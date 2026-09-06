#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <initializer_list>

#ifndef MAYHEMDEBUGGER_API
#define MAYHEMDEBUGGER_API
#endif

#ifndef MDBG_ENABLED
#define MDBG_ENABLED 1
#endif

namespace mdbg {

constexpr int kMaxKeyLength           = 32;
constexpr int kMaxNameLength          = 32;
constexpr int kMaxValueStringLength   = 48;

inline void CopyStr(char* dst, size_t dstSize, const char* src) {
    if (!src || dstSize == 0) { if (dstSize > 0) dst[0] = '\0'; return; }
    size_t i = 0;
    for (; i + 1 < dstSize && src[i] != '\0'; ++i) dst[i] = src[i];
    dst[i] = '\0';
}

enum class ValueType : uint8_t { Bool, Int, Float, String };

struct NamedValue {
    char      name[kMaxNameLength] = {};
    ValueType type = ValueType::Bool;
    union { bool b; int i; float f; };
    char str[kMaxValueStringLength] = {};
    NamedValue() : b(false) {}
};

inline NamedValue V(const char* name, bool        value) { NamedValue v; CopyStr(v.name, sizeof(v.name), name); v.type = ValueType::Bool;   v.b = value; return v; }
inline NamedValue V(const char* name, int         value) { NamedValue v; CopyStr(v.name, sizeof(v.name), name); v.type = ValueType::Int;    v.i = value; return v; }
inline NamedValue V(const char* name, float       value) { NamedValue v; CopyStr(v.name, sizeof(v.name), name); v.type = ValueType::Float;  v.f = value; return v; }
inline NamedValue V(const char* name, const char* value) { NamedValue v; CopyStr(v.name, sizeof(v.name), name); v.type = ValueType::String; CopyStr(v.str, sizeof(v.str), value); return v; }

constexpr int kMaxValuesPerStep = 4;
constexpr int kMaxStepsPerChain = 16;

struct ChainStep {
    char       name[kMaxNameLength] = {};
    bool       passed = false;
    NamedValue values[kMaxValuesPerStep];
    uint8_t    valueCount = 0;
    const char* file = nullptr;
    int        line = 0;
};

struct Chain {
    char      key[kMaxKeyLength] = {};
    ChainStep steps[kMaxStepsPerChain];
    uint8_t   stepCount = 0;
    bool      failed = false;
    uint64_t  sequence = 0;
};

class MAYHEMDEBUGGER_API ChainScope {
public:
    explicit ChainScope(const char* key);
    ~ChainScope();

    ChainScope(const ChainScope&)            = delete;
    ChainScope& operator=(const ChainScope&) = delete;

    bool Check(const char* name, bool condition,
               std::initializer_list<NamedValue> values,
               const char* file, int line);

    bool CheckN(const char* name, bool condition,
                const NamedValue* values, uint8_t valueCount,
                const char* file, int line);

private:
    Chain chain_;
};

MAYHEMDEBUGGER_API ChainScope* CurrentChainScope();

constexpr int kMaxBreakWatches = 8;

MAYHEMDEBUGGER_API void SetBreakOnFail(const char* chainKey, const char* stepName, bool enabled = true);
MAYHEMDEBUGGER_API bool ConsumeBreak();

using BreakHandler = void (*)(const char* chainKey, const char* stepName);
MAYHEMDEBUGGER_API void SetBreakHandler(BreakHandler handler);

} // namespace mdbg

#if defined(_MSC_VER)
    #define MDBG_DEBUG_BREAK() __debugbreak()
#elif defined(__has_builtin)
    #if __has_builtin(__builtin_debugtrap)
        #define MDBG_DEBUG_BREAK() __builtin_debugtrap()
    #else
        #include <csignal>
        #define MDBG_DEBUG_BREAK() std::raise(SIGTRAP)
    #endif
#elif defined(__GNUC__)
    #include <csignal>
    #define MDBG_DEBUG_BREAK() std::raise(SIGTRAP)
#else
    #define MDBG_DEBUG_BREAK() ((void)0)
#endif

#if MDBG_ENABLED

#define MDBG_CONCAT_INNER(a, b) a##b
#define MDBG_CONCAT(a, b) MDBG_CONCAT_INNER(a, b)

#define DEBUG_CHAIN(key) ::mdbg::ChainScope MDBG_CONCAT(_mdbg_scope_, __LINE__)(key)

#define DEBUG_CHECK(name, cond, ...)                                           \
    (::mdbg::CurrentChainScope()                                               \
         ? ::mdbg::CurrentChainScope()->Check((name), (cond), {__VA_ARGS__},   \
                                              __FILE__, __LINE__)               \
         : (cond))

#else

#define DEBUG_CHAIN(key)          ((void)0)
#define DEBUG_CHECK(name, cond, ...) (cond)

#endif
