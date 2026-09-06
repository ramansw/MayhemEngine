#pragma once
// NetTrace extension for MayhemDebugger.
// Include this alongside MayhemDebugger.h when you need network event tracing.
//
// UE:    #include "MayhemDebuggerNetTrace.h"
// Usage: NET_TRACE_SEND("RPC_MovePlayer").Bytes(64).Value("playerId", id).Record();
//        NET_TRACE_RECEIVE("RPC_DamageApplied").Bytes(16).Value("dmg", amount).Record();
#include "ntr/event_log.h"
