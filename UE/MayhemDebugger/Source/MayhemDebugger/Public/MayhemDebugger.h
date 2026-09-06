#pragma once
// Main include for MayhemDebugger.
// Add "MayhemDebugger" to your module's PublicDependencyModuleNames in
// Build.cs, then add this to your module's PCH (YourGame.h) so every .cpp
// gets DEBUG_CHAIN / DEBUG_CHECK / V() with no per-file include needed:
//
//   #include "MayhemDebugger.h"
//
// Then in any .cpp:
//   DEBUG_CHAIN("CanAttack");
//   DEBUG_CHECK("HasTarget", Target != nullptr);
//   DEBUG_CHECK("InRange", Dist <= Range, V("dist", Dist), V("range", Range));

#include "mdbg/chain.h"
#include "mdbg/registry.h"
