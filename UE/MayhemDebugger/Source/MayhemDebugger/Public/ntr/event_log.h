#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>

#ifndef MAYHEMDEBUGGER_API
#define MAYHEMDEBUGGER_API
#endif

// NetTrace — embedded into MayhemDebugger.
// Usage:  #include "MayhemDebuggerNetTrace.h"
// Macros: NET_TRACE_SEND("Name").Bytes(n).Value("k", v).Record();
//         NET_TRACE_RECEIVE("Name").Bytes(n).Record();

namespace ntr {

enum class Direction : int { Send = 0, Receive = 1 };

constexpr int    kMaxValues       = 4;
constexpr int    kMaxNameLen      = 32;
constexpr int    kMaxFormattedLen = 48;
constexpr size_t kRingCapacity    = 512;

struct Value {
    char name[kMaxNameLen];
    char formatted[kMaxFormattedLen];
};

struct NetworkEvent {
    uint64_t  timestampMs;
    uint32_t  sizeBytes;
    Direction direction;
    int       valueCount;
    char      name[kMaxNameLen];
    Value     values[kMaxValues];
};

class MAYHEMDEBUGGER_API EventBuilder {
public:
    EventBuilder(Direction dir, const char* name) noexcept {
        evt_ = {};
        evt_.direction = dir;
        // Use CopyStr-style safe copy; avoids MSVC C4996 strncpy warning
        for (int i = 0; i < kMaxNameLen - 1 && name[i]; ++i) evt_.name[i] = name[i];
    }

    EventBuilder& Bytes(uint32_t n) noexcept { evt_.sizeBytes = n; return *this; }

    template<typename T>
    EventBuilder& Value(const char* key, T val) noexcept {
        if (evt_.valueCount < kMaxValues) {
            auto& v = evt_.values[evt_.valueCount++];
            for (int i = 0; i < kMaxNameLen - 1 && key[i]; ++i) v.name[i] = key[i];
            FormatValue(v.formatted, val);
        }
        return *this;
    }

    void Record() noexcept;

private:
    NetworkEvent evt_;

    static void FormatValue(char* buf, int         v) noexcept { snprintf(buf, kMaxFormattedLen, "%d",   v); }
    static void FormatValue(char* buf, float        v) noexcept { snprintf(buf, kMaxFormattedLen, "%.2f", v); }
    static void FormatValue(char* buf, double       v) noexcept { snprintf(buf, kMaxFormattedLen, "%.2f", v); }
    static void FormatValue(char* buf, bool         v) noexcept { snprintf(buf, kMaxFormattedLen, "%s",   v ? "true" : "false"); }
    static void FormatValue(char* buf, const char*  v) noexcept {
        int i = 0;
        for (; i < kMaxFormattedLen - 1 && v[i]; ++i) buf[i] = v[i];
        buf[i] = '\0';
    }
};

MAYHEMDEBUGGER_API void         RecordEvent(const NetworkEvent& evt) noexcept;
MAYHEMDEBUGGER_API size_t       GetEventCount() noexcept;
MAYHEMDEBUGGER_API NetworkEvent GetEventAt(size_t idx) noexcept;

MAYHEMDEBUGGER_API uint64_t GetTotalBytesSent()      noexcept;
MAYHEMDEBUGGER_API uint64_t GetTotalBytesReceived()  noexcept;
MAYHEMDEBUGGER_API uint64_t GetTotalEventsSent()     noexcept;
MAYHEMDEBUGGER_API uint64_t GetTotalEventsReceived() noexcept;

MAYHEMDEBUGGER_API void ResetForTesting() noexcept;

} // namespace ntr

#define NET_TRACE_SEND(name)    ntr::EventBuilder(ntr::Direction::Send,    name)
#define NET_TRACE_RECEIVE(name) ntr::EventBuilder(ntr::Direction::Receive, name)
