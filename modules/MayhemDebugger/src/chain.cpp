#include "mdbg/chain.h"
#include "mdbg/registry.h"

#include <vector>
#include <atomic>

namespace mdbg {

namespace {
thread_local std::vector<ChainScope*> g_chainStack;
std::atomic<uint64_t> g_sequence{0};
} // namespace

ChainScope::ChainScope(const char* key) {
    chain_.key = key;
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
    if (chain_.stepCount < kMaxStepsPerChain) {
        ChainStep& step = chain_.steps[chain_.stepCount++];
        step.name = name;
        step.passed = condition;
        step.file = file;
        step.line = line;

        uint8_t n = 0;
        for (const auto& v : values) {
            if (n >= kMaxValuesPerStep) break;
            step.values[n++] = v;
        }
        step.valueCount = n;
    }
    if (!condition) {
        chain_.failed = true;
    }
    return condition;
}

ChainScope* CurrentChainScope() {
    return g_chainStack.empty() ? nullptr : g_chainStack.back();
}

} // namespace mdbg
