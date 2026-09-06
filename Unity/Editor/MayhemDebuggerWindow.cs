#if UNITY_EDITOR
using UnityEditor;
using UnityEngine;

namespace MayhemDebugger.Editor
{
    /// <summary>
    /// Live viewer for MDBG chain data. Same idea as the
    /// C++ ImGui overlay (modules/MayhemDebugger/include/mdbg/imgui_overlay.h)
    /// re-expressed as a Unity EditorWindow: one foldout per chain key, green
    /// PASS / red FAIL rows in recorded order, stopping at the first failure
    /// since nothing after that point actually ran.
    ///
    /// Open via Window > MayhemDebugger while in Play Mode -- chains are only
    /// recorded while the game is running (MonoBehaviours calling
    /// MDBG.BeginChain/Check), so the window is empty in Edit
    /// Mode. It repaints every editor tick so it stays live without needing
    /// you to click back into it.
    /// </summary>
    public class MayhemDebuggerWindow : EditorWindow
    {
        private Vector2 _scroll;
        private readonly System.Collections.Generic.Dictionary<string, bool> _expanded =
            new System.Collections.Generic.Dictionary<string, bool>();

        [MenuItem("Window/MayhemDebugger")]
        public static void ShowWindow()
        {
            var window = GetWindow<MayhemDebuggerWindow>();
            window.titleContent = new GUIContent("MayhemDebugger");
            window.Show();
        }

        private void OnEnable()
        {
            EditorApplication.update += Repaint;
        }

        private void OnDisable()
        {
            EditorApplication.update -= Repaint;
        }

        private void OnGUI()
        {
            if (!Application.isPlaying)
            {
                EditorGUILayout.HelpBox(
                    "MayhemDebugger only records chains while the game is running. " +
                    "Enter Play Mode to see live decision chains here.",
                    MessageType.Info);
                return;
            }

            MDBG.ChainInfo[] chains;
            try
            {
                chains = MDBG.SnapshotAllChains();
            }
            catch (System.DllNotFoundException)
            {
                EditorGUILayout.HelpBox(
                    "MayhemDebuggerUnityBridge native plugin not found. Build the " +
                    "MayhemDebuggerUnityBridge CMake target and copy the resulting " +
                    "shared library into this project's Assets/Plugins/ folder.",
                    MessageType.Warning);
                return;
            }

            if (chains == null || chains.Length == 0)
            {
                EditorGUILayout.HelpBox(
                    "No chains recorded yet. Wrap gameplay logic in " +
                    "MDBG.BeginChain(\"...\") / .Check(...) to see it here.",
                    MessageType.Info);
                return;
            }

            _scroll = EditorGUILayout.BeginScrollView(_scroll);

            foreach (var chain in chains)
            {
                if (!_expanded.TryGetValue(chain.Key, out bool isOpen))
                {
                    isOpen = true; // default open -- matches the ImGui overlay's default-visible tree nodes
                }

                string headerLabel = chain.Key + (chain.Failed ? "  [FAILED]" : "  [OK]");
                bool newOpen = EditorGUILayout.Foldout(isOpen, headerLabel, true);
                _expanded[chain.Key] = newOpen;
                if (!newOpen)
                {
                    continue;
                }

                EditorGUI.indentLevel++;
                foreach (var step in chain.Steps)
                {
                    var prevColor = GUI.color;
                    GUI.color = step.Passed ? new Color(0.4f, 0.9f, 0.4f) : new Color(0.95f, 0.35f, 0.35f);
                    EditorGUILayout.LabelField(string.Format("{0}   [{1}]", step.Name, step.Passed ? "PASS" : "FAIL"));
                    GUI.color = prevColor;

                    EditorGUI.indentLevel++;
                    if (step.Values != null)
                    {
                        foreach (var (name, formatted) in step.Values)
                        {
                            EditorGUILayout.LabelField(string.Format("{0} = {1}", name, formatted));
                        }
                    }
                    EditorGUI.indentLevel--;

                    // Steps after the first failure weren't reached -- don't
                    // draw them as if they ran, same rule as the ImGui overlay.
                    if (!step.Passed)
                    {
                        break;
                    }
                }
                EditorGUI.indentLevel--;
                EditorGUILayout.Space(6);
            }

            EditorGUILayout.EndScrollView();
        }
    }
}
#endif
