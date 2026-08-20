// Jade Engine - entry point.
// Phase 1 minimum: open a window, clear it each frame, quit on ESC.

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "core/GLDebug.h"
#include "core/Logger.h"
#include "core/Window.h"

int main() {
    using namespace jade;

    Window window(WindowProps{"Jade Engine", 1280, 720});
    JADE_LOG_INFO("Jade Engine initialized");

    // The render/update loop. In Phase 1 there is no fixed timestep yet -
    // Timer.h (next module) will introduce delta time and a step accumulator.
    while (!window.shouldClose()) {
        window.pollEvents();

        // Phase 1 input is inline; Phase 1's Input module will replace this.
        if (glfwGetKey(window.native(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            window.requestClose();
        }

        // Clear color + depth so we don't accumulate previous frames.
        GL_CHECK(glClearColor(0.10f, 0.10f, 0.12f, 1.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        window.swapBuffers();
    }

    JADE_LOG_INFO("Jade Engine shutdown");
    return 0;
}
