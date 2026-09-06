#include "ntr/event_log.h"
#include <chrono>
#include <mutex>
#include <atomic>

namespace ntr {

namespace {

static uint64_t NowMs() noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

struct RingBuffer {
    NetworkEvent events[kRingCapacity];
    size_t       head   = 0;
    size_t       count  = 0;
    std::mutex   lock;
};

static RingBuffer& Ring() noexcept {
    static RingBuffer r;
    return r;
}

static std::atomic<uint64_t> g_bytesSent     {0};
static std::atomic<uint64_t> g_bytesReceived {0};
static std::atomic<uint64_t> g_eventsSent    {0};
static std::atomic<uint64_t> g_eventsReceived{0};

} // namespace

void EventBuilder::Record() noexcept {
    evt_.timestampMs = NowMs();
    RecordEvent(evt_);
}

void RecordEvent(const NetworkEvent& evt) noexcept {
    if (evt.direction == Direction::Send) {
        g_bytesSent.fetch_add(evt.sizeBytes, std::memory_order_relaxed);
        g_eventsSent.fetch_add(1,            std::memory_order_relaxed);
    } else {
        g_bytesReceived.fetch_add(evt.sizeBytes, std::memory_order_relaxed);
        g_eventsReceived.fetch_add(1,            std::memory_order_relaxed);
    }

    auto& r = Ring();
    std::lock_guard<std::mutex> guard(r.lock);
    r.events[r.head % kRingCapacity] = evt;
    r.head++;
    if (r.count < kRingCapacity) r.count++;
}

size_t GetEventCount() noexcept {
    auto& r = Ring();
    std::lock_guard<std::mutex> guard(r.lock);
    return r.count;
}

NetworkEvent GetEventAt(size_t idx) noexcept {
    auto& r = Ring();
    std::lock_guard<std::mutex> guard(r.lock);
    size_t start = (r.count < kRingCapacity) ? 0 : (r.head % kRingCapacity);
    return r.events[(start + idx) % kRingCapacity];
}

uint64_t GetTotalBytesSent()      noexcept { return g_bytesSent.load(std::memory_order_relaxed); }
uint64_t GetTotalBytesReceived()  noexcept { return g_bytesReceived.load(std::memory_order_relaxed); }
uint64_t GetTotalEventsSent()     noexcept { return g_eventsSent.load(std::memory_order_relaxed); }
uint64_t GetTotalEventsReceived() noexcept { return g_eventsReceived.load(std::memory_order_relaxed); }

void ResetForTesting() noexcept {
    auto& r = Ring();
    std::lock_guard<std::mutex> guard(r.lock);
    r.head  = 0;
    r.count = 0;
    g_bytesSent.store(0,     std::memory_order_relaxed);
    g_bytesReceived.store(0, std::memory_order_relaxed);
    g_eventsSent.store(0,    std::memory_order_relaxed);
    g_eventsReceived.store(0,std::memory_order_relaxed);
}

} // namespace ntr
