// Jade Engine - entry point.
// Phase 1: window + timer + input + a colored mesh drawn each frame.

#include <cstdint>
#include <string>

#include "core/Input.h"
#include "core/Logger.h"
#include "core/Timer.h"
#include "core/Window.h"
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

out vec3 vColor;
out vec2 vTexCoord;

void main() {
    vColor = aColor;
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0);
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
                // TODO(jade): Phase 5 gameplay systems consume the fixed step
            }

            renderer.beginFrame();
            renderer.draw(triangle, shader, white);

            window.swapBuffers();
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
    }
}
