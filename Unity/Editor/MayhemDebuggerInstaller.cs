using UnityEditor;
using UnityEngine;
using System.IO;

namespace MayhemDebugger.Editor
{
    [InitializeOnLoad]
    static class MayhemDebuggerInstaller
    {
        const string GlobalUsingsPath = "Assets/MayhemDebuggerGlobalUsings.cs";
        const string CscRspPath      = "Assets/csc.rsp";

        static MayhemDebuggerInstaller()
        {
            bool changed = false;
            changed |= EnsureGlobalUsings();
            changed |= EnsureCscRsp();
            if (changed)
                AssetDatabase.Refresh();
        }

        static bool EnsureGlobalUsings()
        {
            // csc.rsp (created below) is still needed for C# 10.
            // No global using is injected — add `using MayhemDebugger;` to
            // each script that uses the debugger and call MDBG.Check() etc.
            return false;
        }

        static bool EnsureCscRsp()
        {
            const string flag = "-langversion:10";

            if (File.Exists(CscRspPath))
            {
                string existing = File.ReadAllText(CscRspPath);
                // If any langversion is already set, leave it alone.
                if (existing.Contains("-langversion:")) return false;
                File.AppendAllText(CscRspPath, "\n" + flag + "\n");
                return true;
            }

            File.WriteAllText(CscRspPath, flag + "\n");
            return true;
        }
    }
}
