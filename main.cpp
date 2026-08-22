// Jade Engine - entry point.
// Phase 1: window + timer + input + a colored mesh drawn each frame.

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <string>

#include "core/Input.h"
#include "core/Logger.h"
#include "core/Timer.h"
#include "core/Window.h"
#include "renderer/Camera.h"
#include "renderer/Mesh.h"
#include "renderer/Renderer.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"

namespace {

// #version 330 core so the demo compiles on the 3.3 fallback path as well
// as 4.x. Embedded (not files) so CI does not need an assets copy step.
constexpr const char* kTriangleVertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uViewProj;

out vec3 vColor;
out vec2 vTexCoord;

void main() {
    vColor = aColor;
    vTexCoord = aTexCoord;
    gl_Position = uViewProj * vec4(aPosition, 1.0);
}
)";

constexpr const char* kTriangleFragmentSrc = R"(
#version 330 core
in vec3 vColor;
in vec2 vTexCoord;

uniform sampler2D uTexture;

out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0) * texture(uTexture, vTexCoord);
}
)";

} // namespace

int main() {
    using namespace jade;

    try {
        Window   window(WindowProps{"Jade Engine", 1280, 720});
        Timer    timer;
        Input    input(window.native());
        Renderer renderer;

        Shader shader(kTriangleVertexSrc, kTriangleFragmentSrc);

        const Vertex vertices[] = {
            {{ 0.0f,  0.55f, 0.0f}, {1.0f, 0.25f, 0.20f}, {0.5f, 1.0f}},
            {{-0.55f, -0.55f, 0.0f}, {0.20f, 0.85f, 0.30f}, {0.0f, 0.0f}},
            {{ 0.55f, -0.55f, 0.0f}, {0.20f, 0.40f, 1.00f}, {1.0f, 0.0f}},
        };
        const std::uint32_t indices[] = {0, 1, 2};
        Mesh triangle(vertices, 3, indices, 3);

        // Real 1x1 white RGBA upload. Sampling it leaves vertex colors unchanged.
        const unsigned char kWhiteRgba[] = {255, 255, 255, 255};
        Texture white(1, 1, kWhiteRgba);

        // Static camera two units back on +Z, looking at the triangle. The
        // Phase 5 controller will fly it; for now only the aspect tracks the
        // framebuffer so resizing never stretches the scene.
        Camera camera;
        camera.setAspect(static_cast<float>(window.framebufferWidth())
                         / static_cast<float>(window.framebufferHeight()));
        window.setResizeCallback([&camera](int width, int height) {
            if (height > 0) {
                camera.setAspect(static_cast<float>(width) / static_cast<float>(height));
            }
        });

        JADE_LOG_INFO("Jade Engine initialized");

        // Optional frame cap for headless / CI runs: JADE_MAX_FRAMES=N requests
        // a clean close after N frames (unset or 0 = run until the user quits).
        // A deterministic exit beats killing the process with `timeout`, which
        // would skip the shutdown log the CI gate asserts on.
        unsigned long framesRemaining = 0;
        if (const char* frameCap = std::getenv("JADE_MAX_FRAMES")) {
            framesRemaining = std::strtoul(frameCap, nullptr, 10);
            if (framesRemaining > 0) {
                JADE_LOG_INFO(std::string("Frame cap active: JADE_MAX_FRAMES=")
                              + std::to_string(framesRemaining));
            }
        }

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
                // TODO(jade): Phase 5 gameplay systems consume the fixed step
            }

            renderer.setViewProjection(camera.viewProjection());
            renderer.beginFrame();
            renderer.draw(triangle, shader, white);

            window.swapBuffers();

            if (framesRemaining > 0 && --framesRemaining == 0) {
                window.requestClose();
            }
        }

        const MousePosition mouse = input.mousePosition();
        JADE_LOG_INFO(std::string("Input mouse=(") + std::to_string(mouse.x)
                      + ", " + std::to_string(mouse.y) + ")");
        JADE_LOG_INFO(std::string("Timer totalTime=") + std::to_string(timer.totalTime())
                      + "s fixedDelta=" + std::to_string(timer.fixedDelta()) + "s");
        JADE_LOG_INFO("Jade Engine shutdown");
        return 0;
    } catch (const WindowError& error) {
        JADE_LOG_ERROR(std::string("Failed to start Jade Engine: ") + error.what());
        return 1;
    } catch (const ShaderError& error) {
        JADE_LOG_ERROR(std::string("Failed to start Jade Engine: ") + error.what());
        return 1;
    } catch (const std::exception& error) {
        // Safety net: anything else (bad_alloc, system_error, a future
        // subsystem's error type) still unwinds, logs, and exits instead of
        // hitting std::terminate with no trace.
        JADE_LOG_ERROR(std::string("Unexpected error: ") + error.what());
        return 1;
    }
}
