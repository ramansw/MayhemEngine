Build the MayhemDebuggerUnityBridge CMake target on Windows and drop the
resulting MayhemDebuggerUnityBridge.dll in this folder (next to this file).
Unity will pick it up automatically as part of the package -- no more manual
copying into Assets/Plugins.

First time you add the .dll, select it in the Project window and in the
Inspector's Plugin importer, make sure "Editor" and "Standalone" (x86_64)
are checked under Include Platforms, then Apply.
