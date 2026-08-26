// MayhemDebugger overlay sample — the same enemy-attack scenario as
// enemy_demo, but running live in a window so you can drag the player
// around and watch the decision chain change in real time instead of
// reading a one-shot console dump.
//
// Backend: GLFW + OpenGL3, the same pairing Dear ImGui's own examples use.

#include "mdbg/chain.h"
#include "mdbg/registry.h"
#include "mdbg/imgui_overlay.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>

struct Target {
    float x = 0.0f;
    float y = 0.0f;
};

struct EnemyAI {
    float x = 0.0f;
    float y = 0.0f;
    float attackRange = 5.0f;
    float cooldown = 0.0f;
    bool hasLineOfSight = true;

    static float Distance(float ax, float ay, float bx, float by) {
        float dx = ax - bx;
        float dy = ay - by;
        return std::sqrt(dx * dx + dy * dy);
    }

    void TryAttack(Target* target) {
        DEBUG_CHAIN("CanAttack");

        if (!DEBUG_CHECK("TargetAcquired", target != nullptr)) {
            return;
        }

        float distance = Distance(x, y, target->x, target->y);
        if (!DEBUG_CHECK("InAttackRange", distance <= attackRange,
                          mdbg::V("distance", distance), mdbg::V("range", attackRange))) {
            return;
        }

        if (!DEBUG_CHECK("HasLineOfSight", hasLineOfSight)) {
            return;
        }

        if (!DEBUG_CHECK("CooldownReady", cooldown <= 0.0f, mdbg::V("cooldown", cooldown))) {
            return;
        }

        DEBUG_CHECK("AttackExecuted", true);
    }
};

static void GlfwErrorCallback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

int main() {
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(900, 600, "MayhemDebugger -- overlay demo", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    EnemyAI enemy;
    enemy.attackRange = 5.0f;

    Target player;
    player.x = 5.7f; // starts just out of range -- same as the design report's example

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Scenario controls");
        ImGui::TextWrapped("Drag the player toward/away from the enemy (attack range = %.1f) and watch the chain change.", enemy.attackRange);
        ImGui::SliderFloat("Player X", &player.x, 0.0f, 12.0f);
        ImGui::Checkbox("Has line of sight", &enemy.hasLineOfSight);
        ImGui::SliderFloat("Cooldown", &enemy.cooldown, 0.0f, 3.0f);
        ImGui::End();

        enemy.TryAttack(&player);
        mdbg::DrawOverlay();

        ImGui::Render();
        int displayW, displayH;
        glfwGetFramebufferSize(window, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
