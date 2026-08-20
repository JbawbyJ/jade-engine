// Jade Engine - entry point.
// Phase 1 minimum: open a window, clear it each frame, quit on ESC.

#include <glad/glad.h>

#include <string>

#include "core/GLDebug.h"
#include "core/Input.h"
#include "core/Logger.h"
#include "core/Timer.h"
#include "core/Window.h"

int main() {
    using namespace jade;

    Window window(WindowProps{"Jade Engine", 1280, 720});
    Timer  timer;
    Input  input(window.native());
    JADE_LOG_INFO("Jade Engine initialized");

    // Variable-render / fixed-update loop. Simulation work goes inside the
    // consumeFixedStep drain; rendering stays outside and uses deltaTime().
    while (!window.shouldClose()) {
        timer.tick();
        window.pollEvents();
        input.update();

        if (input.isKeyDown(Key::Escape)) {
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

    const MousePosition mouse = input.mousePosition();
    JADE_LOG_INFO(std::string("Input mouse=(") + std::to_string(mouse.x)
                  + ", " + std::to_string(mouse.y) + ")");
    JADE_LOG_INFO(std::string("Timer totalTime=") + std::to_string(timer.totalTime())
                  + "s fixedDelta=" + std::to_string(timer.fixedDelta()) + "s");
    JADE_LOG_INFO("Jade Engine shutdown");
    return 0;
}
