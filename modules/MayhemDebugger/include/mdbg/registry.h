#pragma once
// MayhemDebugger — chain registry. Keeps the most recently completed chain
// for each named key (e.g. "CanAttack"). v0.1 intentionally keeps only the
// latest chain per key — cross-frame history/replay is a stretch goal, not
// part of this scope (see docs/MayhemDebugger_Design_Report.docx, Sec. 7).

#include "mdbg/chain.h"
#include <string>
#include <vector>
#include <functional>

namespace mdbg {

class Registry {
public:
    static Registry& Get();

    void Publish(const Chain& finished);
    const Chain* Latest(const char* key) const;
    void ForEachKey(const std::function<void(const std::string&)>& fn) const;

    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;

private:
    Registry() = default;

    struct Slot {
        std::string key;
        Chain chain;
    };
    std::vector<Slot> slots_; // linear scan — fine for tens of chain keys
};

} // namespace mdbg
