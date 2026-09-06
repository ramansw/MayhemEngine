#pragma once
#include "mdbg/chain.h"
#include <string>
#include <vector>
#include <functional>

#ifndef MAYHEMDEBUGGER_API
#define MAYHEMDEBUGGER_API
#endif

namespace mdbg {

class MAYHEMDEBUGGER_API Registry {
public:
    static Registry& Get();

    void Publish(const Chain& finished);
    const Chain* Latest(const char* key) const;
    void ForEachKey(const std::function<void(const std::string&)>& fn) const;

    Registry(const Registry&)            = delete;
    Registry& operator=(const Registry&) = delete;

private:
    Registry() = default;

    struct Slot {
        std::string key;
        Chain       chain;
    };
    std::vector<Slot> slots_;
};

} // namespace mdbg
