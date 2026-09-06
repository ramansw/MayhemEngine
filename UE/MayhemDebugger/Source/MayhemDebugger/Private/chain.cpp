#include "mdbg/chain.h"
#include "mdbg/registry.h"

#include <vector>
#include <atomic>

namespace mdbg {

namespace {
thread_local std::vector<ChainScope*> g_chainStack;
std::atomic<uint64_t> g_sequence{0};

struct BreakWatch {
    char key[kMaxKeyLength] = {};
    char step[kMaxNameLength] = {};
    bool used = false;
};
BreakWatch g_breakWatches[kMaxBreakWatches];
std::atomic<bool> g_breakPending{false};

void DefaultBreakHandler(const char* /*chainKey*/, const char* /*stepName*/) {
    MDBG_DEBUG_BREAK();
}

BreakHandler g_breakHandler = &DefaultBreakHandler;

bool StrEq(const char* a, const char* b) {
    return std::strcmp(a, b) == 0;
}
} // namespace

ChainScope::ChainScope(const char* key) {
    CopyStr(chain_.key, sizeof(chain_.key), key);
    g_chainStack.push_back(this);
}

ChainScope::~ChainScope() {
    chain_.sequence = g_sequence.fetch_add(1, std::memory_order_relaxed);
    Registry::Get().Publish(chain_);
    g_chainStack.pop_back();
}

bool ChainScope::Check(const char* name, bool condition,
                        std::initializer_list<NamedValue> values,
                        const char* file, int line) {
    NamedValue buf[kMaxValuesPerStep];
    uint8_t n = 0;
    for (const auto& v : values) {
        if (n >= kMaxValuesPerStep) break;
        buf[n++] = v;
    }
    return CheckN(name, condition, buf, n, file, line);
}

bool ChainScope::CheckN(const char* name, bool condition,
                         const NamedValue* values, uint8_t valueCount,
                         const char* file, int line) {
    if (chain_.stepCount < kMaxStepsPerChain) {
        ChainStep& step = chain_.steps[chain_.stepCount++];
        CopyStr(step.name, sizeof(step.name), name);
        step.passed = condition;
        step.file = file;
        step.line = line;

        uint8_t n = (valueCount < kMaxValuesPerStep) ? valueCount : kMaxValuesPerStep;
        for (uint8_t i = 0; i < n; ++i) {
            step.values[i] = values[i];
        }
        step.valueCount = n;
    }
    if (!condition) {
        chain_.failed = true;
        for (const BreakWatch& w : g_breakWatches) {
            if (w.used && StrEq(w.key, chain_.key) && StrEq(w.step, name)) {
                g_breakPending.store(true, std::memory_order_relaxed);
                if (g_breakHandler) {
                    g_breakHandler(chain_.key, name);
                }
                break;
            }
        }
    }
    return condition;
}

ChainScope* CurrentChainScope() {
    return g_chainStack.empty() ? nullptr : g_chainStack.back();
}

void SetBreakOnFail(const char* chainKey, const char* stepName, bool enabled) {
    if (!chainKey || !stepName) return;
    for (BreakWatch& w : g_breakWatches) {
        if (w.used && StrEq(w.key, chainKey) && StrEq(w.step, stepName)) {
            if (enabled) return;
            w.used = false;
            w.key[0] = '\0';
            w.step[0] = '\0';
            return;
        }
    }
    if (!enabled) return;
    for (BreakWatch& w : g_breakWatches) {
        if (!w.used) {
            CopyStr(w.key, sizeof(w.key), chainKey);
            CopyStr(w.step, sizeof(w.step), stepName);
            w.used = true;
            return;
        }
    }
}

bool ConsumeBreak() {
    return g_breakPending.exchange(false, std::memory_order_relaxed);
}

void SetBreakHandler(BreakHandler handler) {
    g_breakHandler = handler ? handler : &DefaultBreakHandler;
}

} // namespace mdbg
