using System;
using System.Runtime.InteropServices;
using UnityEngine;

namespace MayhemDebugger
{
    /// <summary>
    /// Thin C# wrapper around MayhemDebuggerUnityBridge.dll (native side:
    /// modules/MayhemDebuggerUnityBridge in the MayhemEngine repo; underlying
    /// chain-recording logic: modules/MayhemDebugger).
    ///
    /// Setup: build the MayhemDebuggerUnityBridge CMake target, then copy the
    /// resulting .dll into this Unity project's Assets/Plugins/ folder --
    /// Unity loads native plugins from there automatically.
    ///
    /// Referencing it from a script: exactly like `using UnityEngine;` gets
    /// you `Debug.Log(...)` instead of `UnityEngine.Debug.Log(...)`, one line
    /// -- `using MayhemDebugger;` -- gets you `MDBG.X(...)` instead of the
    /// fully-qualified name. If you don't even want the `MDBG.` prefix at
    /// call sites, add a second line instead:
    /// `using static MayhemDebugger.MDBG;` -- then every method below
    /// (Check, CheckOnce, BeginChain, SetBreakOnFail...) is callable
    /// completely bare, e.g. `CheckOnce("IsAlive", ...)`.
    ///
    /// Don't want to repeat even that one line in every script, in every
    /// project you ever use this in? Copy GlobalUsings.cs (ships right next
    /// to this file) into the project once -- it does the
    /// `using static ...` project-wide, so no script needs any using line
    /// for this at all. See that file for the one requirement (C# 10).
    ///
    /// Two ways to use it:
    ///
    /// 1) Grouped -- several sequential checks that gate one decision, shown
    ///    together in the window with "stopped at step N" when one fails.
    ///    Mirrors the C++ DEBUG_CHAIN/DEBUG_CHECK macros:
    /// <code>
    /// using (MDBG.BeginChain("CanAttack"))
    /// {
    ///     if (!MDBG.Check("TargetAcquired", target != null)) return;
    ///     float distance = Vector3.Distance(transform.position, target.position);
    ///     if (!MDBG.Check("InAttackRange", distance <= attackRange,
    ///             ("distance", distance), ("range", attackRange))) return;
    /// }
    /// </code>
    ///
    /// 2) One-off -- a single condition, no grouping, no `using` block. Drop-in
    ///    replacement for `if (!condition) return;` that also shows up in the
    ///    window as its own one-step entry:
    /// <code>
    /// if (!MDBG.CheckOnce("IsAlive", currentHealth > 0)) return;
    /// </code>
    ///
    /// Break-on-fail -- pause Play mode automatically the moment one specific
    /// named step fails, instead of watching the window and hoping to catch
    /// it. Call once (Awake/Start is fine) to arm a watch:
    /// <code>
    /// MDBG.SetBreakOnFail("CanAttack", "InAttackRange");
    /// </code>
    /// From then on, every Check()/CheckOnce() call for that exact chain key
    /// + step name will call UnityEngine.Debug.Break() the instant it fails.
    /// This is implemented in the shared C++ core (modules/MayhemDebugger),
    /// not just here -- the console app and the ImGui overlay get the same
    /// watch mechanism, just with a real native debugger trap instead of
    /// Debug.Break() (see MDBG_DEBUG_BREAK() in chain.h). Unity can't safely
    /// take a native trap inside a DLL the Editor loaded, so this bridge
    /// polls mdbg_consume_break() after every check instead and calls
    /// Debug.Break() itself from managed code -- see Check() below.
    /// </summary>
    public static class MDBG
    {
        private const string DllName = "MayhemDebuggerUnityBridge";

        [DllImport(DllName, CharSet = CharSet.Ansi)]
        private static extern void mdbg_begin_chain(string key);

        [DllImport(DllName)]
        private static extern void mdbg_end_chain();

        [DllImport(DllName, CharSet = CharSet.Ansi)]
        private static extern int mdbg_check(string name, int condition,
            string[] valueNames, string[] valueFormatted, int valueCount);

        [DllImport(DllName)]
        private static extern int mdbg_chain_count();

        [DllImport(DllName)]
        private static extern IntPtr mdbg_chain_key_at(int index);

        [DllImport(DllName, CharSet = CharSet.Ansi)]
        private static extern int mdbg_chain_step_count(string key);

        [DllImport(DllName, CharSet = CharSet.Ansi)]
        private static extern int mdbg_chain_failed(string key);

        [DllImport(DllName, CharSet = CharSet.Ansi)]
        private static extern IntPtr mdbg_step_name(string key, int stepIndex);

        [DllImport(DllName, CharSet = CharSet.Ansi)]
        private static extern int mdbg_step_passed(string key, int stepIndex);

        [DllImport(DllName, CharSet = CharSet.Ansi)]
        private static extern int mdbg_step_value_count(string key, int stepIndex);

        [DllImport(DllName, CharSet = CharSet.Ansi)]
        private static extern IntPtr mdbg_step_value_name(string key, int stepIndex, int valueIndex);

        [DllImport(DllName, CharSet = CharSet.Ansi)]
        private static extern IntPtr mdbg_step_value_formatted(string key, int stepIndex, int valueIndex);

        [DllImport(DllName, CharSet = CharSet.Ansi)]
        private static extern void mdbg_set_break_on_fail(string chainKey, string stepName, int enabled);

        [DllImport(DllName)]
        private static extern int mdbg_consume_break();

        private static string PtrToString(IntPtr ptr) => ptr == IntPtr.Zero ? null : Marshal.PtrToStringAnsi(ptr);

        /// <summary>
        /// RAII-style chain scope, mirrors the C++ DEBUG_CHAIN macro. Always
        /// pair with `using` so the chain closes even on an early return or
        /// an exception -- exactly like the C++ version closing on scope exit.
        /// </summary>
        public readonly struct ChainScope : IDisposable
        {
            public void Dispose() => mdbg_end_chain();
        }

        public static ChainScope BeginChain(string key)
        {
            mdbg_begin_chain(key);
            return new ChainScope();
        }

        /// <summary>
        /// Records one step. Always returns `condition` unchanged -- exactly
        /// like the C++ DEBUG_CHECK macro, this must never change your
        /// control flow, only observe it. Pass named values as (name, value)
        /// tuples; each value is formatted to a display string on this side
        /// before crossing into native code, so only plain strings/ints ever
        /// cross the P/Invoke boundary. Must be called inside an open
        /// BeginChain(...) scope -- see CheckOnce() below if you don't want
        /// one.
        /// </summary>
        public static bool Check(string name, bool condition, params (string name, object value)[] values)
        {
            int count = values?.Length ?? 0;
            string[] valueNames = count > 0 ? new string[count] : null;
            string[] valueFormatted = count > 0 ? new string[count] : null;
            for (int i = 0; i < count; i++)
            {
                valueNames[i] = values[i].name;
                valueFormatted[i] = FormatValue(values[i].value);
            }
            bool result = mdbg_check(name, condition ? 1 : 0, valueNames, valueFormatted, count) != 0;

            // Break-on-fail: if this exact chain key + step name is watched
            // (see SetBreakOnFail below) and it just failed, the native core
            // already flagged a pending break -- poll it and pause Play mode
            // right here, as close to the failing call site as Unity's
            // managed side can get. Cheap when nothing is watched: one
            // P/Invoke call returning 0.
            if (mdbg_consume_break() != 0)
            {
                Debug.Break();
            }

            return result;
        }

        /// <summary>
        /// One-line convenience for a standalone check that doesn't need step
        /// grouping: opens a fresh one-step chain keyed by `name` itself,
        /// records this one condition, and closes it immediately -- no
        /// `using (BeginChain(...))` wrapper needed at the call site. Drop-in
        /// replacement for `if (!condition) return;`:
        /// <code>
        /// if (!MDBG.CheckOnce("IsAlive", currentHealth > 0)) return;
        /// </code>
        /// Shows up in the window as its own entry named `name`. Because each
        /// call opens and closes its own chain, two CheckOnce calls that pass
        /// the SAME name will overwrite each other in the window (the
        /// registry only keeps the latest chain per key) -- if you have two
        /// or more checks in the same method that you want visible at the
        /// same time, either give each a distinct name, or use
        /// BeginChain(...) + Check(...) instead so they're grouped under one
        /// key and show together, in order, with "stopped here" on failure.
        /// </summary>
        public static bool CheckOnce(string name, bool condition, params (string name, object value)[] values)
        {
            using (BeginChain(name))
            {
                return Check(name, condition, values);
            }
        }

        /// <summary>
        /// Arms (or disarms) an automatic pause: the moment the step named
        /// `stepName`, inside the chain keyed `chainKey`, next fails via
        /// Check() or CheckOnce(), Play mode pauses (UnityEngine.Debug.Break())
        /// -- no manually placed breakpoint, no watching the window hoping to
        /// catch it. Matching is on the exact (chainKey, stepName) pair, so
        /// this is safe to call for one specific failure you're chasing
        /// without pausing on every other check in the project.
        /// <code>
        /// void Awake() => MDBG.SetBreakOnFail("CanAttack", "InAttackRange");
        /// </code>
        /// Call again with `enabled: false` to stop watching that pair. The
        /// watch lives in the shared C++ core (see chain.h/chain.cpp), not
        /// in this file -- every front end built on MayhemDebugger gets it,
        /// this bridge is just the one that turns it into Debug.Break().
        /// </summary>
        public static void SetBreakOnFail(string chainKey, string stepName, bool enabled = true)
        {
            mdbg_set_break_on_fail(chainKey, stepName, enabled ? 1 : 0);
        }

        private static string FormatValue(object value)
        {
            switch (value)
            {
                case float f: return f.ToString("0.00");
                case double d: return d.ToString("0.00");
                case bool b: return b ? "true" : "false";
                default: return value?.ToString() ?? "";
            }
        }

        public struct ChainStepInfo
        {
            public string Name;
            public bool Passed;
            public (string name, string formatted)[] Values;
        }

        public struct ChainInfo
        {
            public string Key;
            public bool Failed;
            public ChainStepInfo[] Steps;
        }

        /// <summary>Snapshot of every currently recorded chain, for a debug window to draw.</summary>
        public static ChainInfo[] SnapshotAllChains()
        {
            int chainCount = mdbg_chain_count();
            var result = new ChainInfo[chainCount];
            for (int c = 0; c < chainCount; c++)
            {
                string key = PtrToString(mdbg_chain_key_at(c));
                int stepCount = mdbg_chain_step_count(key);
                var steps = new ChainStepInfo[Math.Max(stepCount, 0)];
                for (int s = 0; s < stepCount; s++)
                {
                    int valueCount = mdbg_step_value_count(key, s);
                    var stepValues = new (string, string)[Math.Max(valueCount, 0)];
                    for (int v = 0; v < valueCount; v++)
                    {
                        stepValues[v] = (PtrToString(mdbg_step_value_name(key, s, v)),
                                         PtrToString(mdbg_step_value_formatted(key, s, v)));
                    }
                    steps[s] = new ChainStepInfo
                    {
                        Name = PtrToString(mdbg_step_name(key, s)),
                        Passed = mdbg_step_passed(key, s) != 0,
                        Values = stepValues,
                    };
                }
                result[c] = new ChainInfo { Key = key, Failed = mdbg_chain_failed(key) != 0, Steps = steps };
            }
            return result;
        }
    }
}
