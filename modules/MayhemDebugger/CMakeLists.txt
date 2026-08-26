add_library(MayhemDebugger STATIC
    src/chain.cpp
    src/registry.cpp
)

target_include_directories(MayhemDebugger PUBLIC include)
target_compile_features(MayhemDebugger PUBLIC cxx_std_17)
target_compile_definitions(MayhemDebugger PUBLIC MDBG_ENABLED=$<BOOL:${MDBG_ENABLED}>)

add_subdirectory(samples/enemy_demo)

# ---------------------------------------------------------------------------
# Optional ImGui overlay (Milestone M2).
#
# Kept fully separate from the MayhemDebugger target above: the core chain
# recorder + registry has zero rendering dependencies, matching the
# architecture in docs/MayhemDebugger_Design_Report.docx ("core does not
# depend on ImGui"). Only MayhemDebuggerOverlay and overlay_demo touch
# ImGui/GLFW/OpenGL.
#
# First configure needs internet access -- FetchContent clones Dear ImGui
# and GLFW into build/_deps the first time you run cmake. Set
# MDBG_WITH_IMGUI=OFF if you want a pure console build with no GUI deps.
# ---------------------------------------------------------------------------
option(MDBG_WITH_IMGUI "Build the MayhemDebugger ImGui overlay + windowed demo" ON)

if (MDBG_WITH_IMGUI)
    include(FetchContent)

    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.90.9
    )

    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )

    FetchContent_MakeAvailable(imgui glfw)

    find_package(OpenGL REQUIRED)

    # Dear ImGui doesn't ship a CMakeLists.txt for core+backends, so build
    # the handful of files this project actually needs ourselves.
    add_library(imgui_glfw_opengl3 STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui_glfw_opengl3 PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui_glfw_opengl3 PUBLIC glfw OpenGL::GL)

    add_library(MayhemDebuggerOverlay STATIC
        src/imgui_overlay.cpp
    )
    target_include_directories(MayhemDebuggerOverlay PUBLIC include)
    target_link_libraries(MayhemDebuggerOverlay PUBLIC MayhemDebugger imgui_glfw_opengl3)

    add_subdirectory(samples/overlay_demo)
endif()
