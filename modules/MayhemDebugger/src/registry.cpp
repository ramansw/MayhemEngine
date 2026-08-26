#include "mdbg/registry.h"

namespace mdbg {

Registry& Registry::Get() {
    static Registry instance;
    return instance;
}

void Registry::Publish(const Chain& finished) {
    for (auto& slot : slots_) {
        if (slot.key == finished.key) {
            slot.chain = finished;
            return;
        }
    }
    slots_.push_back(Slot{ std::string(finished.key), finished });
}

const Chain* Registry::Latest(const char* key) const {
    for (const auto& slot : slots_) {
        if (slot.key == key) {
            return &slot.chain;
        }
    }
    return nullptr;
}

void Registry::ForEachKey(const std::function<void(const std::string&)>& fn) const {
    for (const auto& slot : slots_) {
        fn(slot.key);
    }
}

} // namespace mdbg
