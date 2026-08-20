// Jade Engine - entry point.
// Phase 1 minimum: open a window, clear it each frame, quit on ESC.

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

#include "core/GLDebug.h"
#include "core/Logger.h"
#include "core/Timer.h"
#include "core/Window.h"

int main() {
    using namespace jade;

    Window window(WindowProps{"Jade Engine", 1280, 720});
    Timer  timer;
    JADE_LOG_INFO("Jade Engine initialized");

    // Variable-render / fixed-update loop. Simulation work goes inside the
    // consumeFixedStep drain; rendering stays outside and uses deltaTime().
    while (!window.shouldClose()) {
        timer.tick();
        window.pollEvents();

        // TODO(jade): replace with Input module
        if (glfwGetKey(window.native(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            window.requestClose();
        }

        while (timer.consumeFixedStep()) {
            // Fixed-rate simulation hook. Empty until gameplay systems land.
        }

        // Clear color + depth so we don't accumulate previous frames.
        GL_CHECK(glClearColor(0.10f, 0.10f, 0.12f, 1.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        window.swapBuffers();
    }

    // Prove the clock advanced during the run (useful for headless smoke tests).
    JADE_LOG_INFO(std::string("Timer totalTime=") + std::to_string(timer.totalTime())
                  + "s fixedDelta=" + std::to_string(timer.fixedDelta()) + "s");
    JADE_LOG_INFO("Jade Engine shutdown");
    return 0;
}
