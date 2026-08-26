// MayhemDebugger sample — reproduces the "enemy isn't attacking" scenario
// from the design report: a decision chain that fails at the range check,
// dumped to the console instead of guessed at with print statements.

#include "mdbg/chain.h"
#include "mdbg/registry.h"

#include <cstdio>
#include <cmath>

struct Target {
    float x = 0.0f;
    float y = 0.0f;
};

struct EnemyAI {
    float x = 0.0f;
    float y = 0.0f;
    float attackRange = 5.0f;
    float cooldown = 0.0f;
    bool hasLineOfSight = true;

    static float Distance(float ax, float ay, float bx, float by) {
        float dx = ax - bx;
        float dy = ay - by;
        return std::sqrt(dx * dx + dy * dy);
    }

    void TryAttack(Target* target) {
        DEBUG_CHAIN("CanAttack");

        if (!DEBUG_CHECK("TargetAcquired", target != nullptr)) {
            return;
        }

        float distance = Distance(x, y, target->x, target->y);
        if (!DEBUG_CHECK("InAttackRange", distance <= attackRange,
                          mdbg::V("distance", distance), mdbg::V("range", attackRange))) {
            return; // <- this is the step the design report's example fails on
        }

        if (!DEBUG_CHECK("HasLineOfSight", hasLineOfSight)) {
            return;
        }

        if (!DEBUG_CHECK("CooldownReady", cooldown <= 0.0f, mdbg::V("cooldown", cooldown))) {
            return;
        }

        std::printf("  >> Enemy attacks!\n");
        DEBUG_CHECK("AttackExecuted", true);
    }
};

static const char* FormatValue(const mdbg::NamedValue& v, char* buf, size_t bufSize) {
    switch (v.type) {
        case mdbg::ValueType::Bool:   std::snprintf(buf, bufSize, "%s", v.b ? "true" : "false"); break;
        case mdbg::ValueType::Int:    std::snprintf(buf, bufSize, "%d", v.i); break;
        case mdbg::ValueType::Float:  std::snprintf(buf, bufSize, "%.2f", v.f); break;
        case mdbg::ValueType::String: std::snprintf(buf, bufSize, "%s", v.str ? v.str : ""); break;
    }
    return buf;
}

static void DumpChain(const char* key) {
    const mdbg::Chain* chain = mdbg::Registry::Get().Latest(key);
    if (!chain) {
        std::printf("[%s] no recorded chain yet\n", key);
        return;
    }
    std::printf("== %s ==\n", chain->key);
    for (uint8_t i = 0; i < chain->stepCount; ++i) {
        const mdbg::ChainStep& step = chain->steps[i];
        std::printf("  %-16s [%s]", step.name, step.passed ? "PASS" : "FAIL");
        for (uint8_t v = 0; v < step.valueCount; ++v) {
            char buf[32];
            std::printf("  %s=%s", step.values[v].name, FormatValue(step.values[v], buf, sizeof(buf)));
        }
        std::printf("\n");
        if (!step.passed) {
            std::printf("  -- chain stopped here, remaining steps not reached --\n");
            break;
        }
    }
}

int main() {
    EnemyAI enemy;
    enemy.x = 0.0f;
    enemy.y = 0.0f;
    enemy.attackRange = 5.0f;

    Target player;
    player.x = 5.7f; // just out of range -> reproduces the design report's bug
    player.y = 0.0f;

    std::printf("-- Attempt 1: player just out of range --\n");
    enemy.TryAttack(&player);
    DumpChain("CanAttack");

    std::printf("\n-- Attempt 2: player moves into range --\n");
    player.x = 3.0f;
    enemy.TryAttack(&player);
    DumpChain("CanAttack");

    return 0;
}
