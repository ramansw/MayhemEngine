using UnityEditor;
using UnityEngine;
using MayhemDebugger.NetTrace;

namespace MayhemDebugger.NetTrace.Editor
{
    public class NetTraceWindow : EditorWindow
    {
        private Vector2 _scroll;

        [MenuItem("Window/MayhemDebugger/NetTrace")]
        public static void ShowWindow()
        {
            var win = GetWindow<NetTraceWindow>("NetTrace");
            win.minSize = new Vector2(400, 280);
        }

        private void OnGUI()
        {
            using (new EditorGUILayout.HorizontalScope(EditorStyles.toolbar))
            {
                GUILayout.Label("MayhemDebugger — NetTrace", EditorStyles.boldLabel);
                GUILayout.FlexibleSpace();
                if (GUILayout.Button("Clear", EditorStyles.toolbarButton))
                    NTR.ResetForTesting();
            }

            EditorGUILayout.Space(4);
            EditorGUILayout.LabelField(
                $"Sent:     {NTR.GetTotalEventsSent()} events  /  {NTR.GetTotalBytesSent()} bytes",
                EditorStyles.miniLabel);
            EditorGUILayout.LabelField(
                $"Received: {NTR.GetTotalEventsReceived()} events  /  {NTR.GetTotalBytesReceived()} bytes",
                EditorStyles.miniLabel);
            EditorGUILayout.Space(4);

            _scroll = EditorGUILayout.BeginScrollView(_scroll);

            int count = NTR.GetEventCount();
            if (count == 0)
            {
                EditorGUILayout.HelpBox(
                    "No events yet.\n" +
                    "Call NTR.RecordSend(\"Name\").Bytes(n).Value(\"key\", v).Record() from code.",
                    MessageType.Info);
            }
            else
            {
                int start = Mathf.Max(0, count - 64);
                for (int i = count - 1; i >= start; i--)
                {
                    var evt = NTR.GetEvent(i);
                    if (evt == null) continue;
                    var e = evt.Value;

                    bool isSend = e.Direction == NTR.Direction.Send;
                    var  color  = isSend
                        ? new Color(0.4f, 0.85f, 1.0f)
                        : new Color(0.4f, 1.0f, 0.6f);

                    var old = GUI.color;
                    GUI.color = color;
                    string dir   = isSend ? "↑ SEND" : "↓ RECV";
                    string bytes = e.SizeBytes > 0 ? $"{e.SizeBytes} B" : "? B";
                    EditorGUILayout.LabelField($"{dir}   {e.Name,-24}   {bytes}",
                        EditorStyles.miniLabel);
                    GUI.color = old;

                    GUI.color = new Color(1f, 0.9f, 0.5f);
                    for (int v = 0; v < e.ValueNames.Length; v++)
                        EditorGUILayout.LabelField($"    {e.ValueNames[v]} = {e.ValueFormatted[v]}",
                            EditorStyles.miniLabel);
                    GUI.color = old;
                }
            }

            EditorGUILayout.EndScrollView();
            Repaint();
        }
    }
}
