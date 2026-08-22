// Jade Engine - entry point.
// Phase 5 demo: fly camera (WASD + Space/LeftShift, right-drag look) over a
// disk-loaded, Lambert-lit scene; simulation runs on the fixed step and
// rendering blends the last two simulation states by Timer::alpha().

#include <cstdlib>
#include <exception>
#include <memory>
#include <string>

#include "assets/AssetError.h"
#include "assets/MeshLoader.h"
#include "assets/ShaderLoader.h"
#include "assets/TextureLoader.h"
#include "core/Input.h"
#include "core/Logger.h"
#include "core/Timer.h"
#include "core/Window.h"
#include "game/CameraController.h"
#include "game/Spinner.h"
#include "math/Transform.h"
#include "renderer/Camera.h"
#include "renderer/DebugDraw.h"
#include "renderer/Mesh.h"
#include "renderer/Renderer.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "scene/Scene.h"

int main() {
    using namespace jade;

    try {
        // Camera is declared BEFORE Window on purpose: the resize callback
        // stored on the window captures it by reference, and destruction runs
        // in reverse order — the camera must outlive the window that might
        // still dispatch into that callback during teardown.
        Camera camera;

        Window   window(WindowProps{"Jade Engine", 1280, 720});

        // Multi-window proof (Phase 6): JADE_SECOND_WINDOW=1 constructs and
        // destroys a second window before any GL resources exist, exercising
        // the GLFW refcount and the context-restore hygiene. Explicitly not
        // an editor shell. The second constructor leaves ITS context current
        // (and reloads glad's process-global pointers from it); destroying it
        // leaves none current, so the primary is restored right after.
        if (const char* secondFlag = std::getenv("JADE_SECOND_WINDOW");
            secondFlag != nullptr && secondFlag[0] == '1') {
            {
                Window second(WindowProps{"Jade Engine (second)", 320, 240});
                JADE_LOG_INFO("Second window constructed; destroying it again");
            }
            window.makeCurrent();
            JADE_LOG_INFO("Second window destroyed; primary context restored");
        }

        Timer     timer;
        Input     input(window.native());
        Renderer  renderer;
        DebugDraw debugDraw; // F1 toggles the grid + axes overlay

        // Everything below loads from disk — Phase 4's whole point. The
        // unique_ptrs are declared after Window so all GL objects die before
        // the context does, and before the Scene so its non-owning pointers
        // never outlive what they point at.
        std::unique_ptr<Shader>  shader    = loadShader("shaders/basic.vert",
                                                        "shaders/basic.frag");
        std::unique_ptr<Texture> checker   = loadTexture("textures/checker.png");
        std::unique_ptr<Mesh>    cubeMesh  = loadMeshObj("meshes/cube.obj");
        std::unique_ptr<Mesh>    planeMesh = loadMeshObj("meshes/plane.obj");

        // Lit ground plane with three cubes resting on it (plane top sits at
        // y = -0.6; each cube's center is that plus half its scaled height).
        // The center and right cubes spin under the fixed timestep.
        Scene scene;
        EntityId spinCenter = 0;
        EntityId spinRight = 0;
        {
            Entity& ground = scene.createEntity("ground");
            ground.mesh = planeMesh.get();
            ground.texture = checker.get();
            ground.transform.position = {0.0f, -0.6f, 0.0f};
            ground.transform.scale = {3.0f, 1.0f, 3.0f};

            Entity& cubeA = scene.createEntity("cube-center");
            cubeA.mesh = cubeMesh.get();
            cubeA.texture = checker.get();
            cubeA.transform.position = {0.0f, -0.1f, 0.0f};
            spinCenter = scene.entities().size() - 1;

            Entity& cubeB = scene.createEntity("cube-left");
            cubeB.mesh = cubeMesh.get();
            cubeB.texture = checker.get();
            cubeB.transform.position = {-1.4f, -0.3f, -0.9f};
            cubeB.transform.rotationEuler.y = 0.5f;
            cubeB.transform.scale = {0.6f, 0.6f, 0.6f};

            Entity& cubeC = scene.createEntity("cube-right");
            cubeC.mesh = cubeMesh.get();
            cubeC.texture = checker.get();
            cubeC.transform.position = {1.3f, -0.35f, 0.6f};
            cubeC.transform.rotationEuler.y = -0.7f;
            cubeC.transform.scale = {0.5f, 0.5f, 0.5f};
            spinRight = scene.entities().size() - 1;
        }
        scene.snapshotPrevious(); // valid previous transforms before frame one

        // Start slightly raised and pulled back, looking gently down at the
        // scene; the controller seeds its authoritative pose from this. Both
        // aspect paths guard height 0 (minimized / unrealized framebuffers).
        camera.setPosition({0.0f, 0.5f, 3.0f});
        camera.setPitch(-0.12f);
        if (window.framebufferHeight() > 0) {
            camera.setAspect(static_cast<float>(window.framebufferWidth())
                             / static_cast<float>(window.framebufferHeight()));
        }
        window.setResizeCallback([&camera](int width, int height) {
            if (height > 0) {
                camera.setAspect(static_cast<float>(width) / static_cast<float>(height));
            }
        });

        // Fly camera (WASD + Space/LeftShift, right-drag look) and the
        // spinning-cube prover, both stepped only inside the fixed drain.
        CameraController controller(camera);
        Spinner spinner(0.8f); // rad/s: clearly moving, not dizzying
        spinner.add(spinCenter);
        spinner.add(spinRight);

        bool showDebugDraw = false;

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

            // Edge query, not a live poll: with sticky input a press+release
            // inside one frame still registers, so a quick ESC tap always quits.
            if (input.wasKeyPressed(Key::Escape)) {
                window.requestClose();
            }
            if (input.wasKeyPressed(Key::F1)) {
                showDebugDraw = !showDebugDraw;
            }

            // Bank this frame's look delta before the drain — mouseDelta()
            // is overwritten next update(), so zero-step frames must not
            // drop it (sensitivity would scale with framerate).
            controller.collectLook(input);

            // Snapshot inside the drain, per CONSUMED step: previous/current
            // must be the two most recent real simulation states. A per-frame
            // snapshot collapses them on zero-step frames and interpolation
            // degenerates into a 60 Hz stair-step (Phase 5 review finding).
            while (timer.consumeFixedStep()) {
                scene.snapshotPrevious();
                controller.snapshotPrevious();
                controller.fixedUpdate(input, timer.fixedDelta());
                spinner.fixedUpdate(scene, timer.fixedDelta());
            }

            const float alpha = timer.alpha();
            controller.writeToCamera(alpha);
            renderer.setViewProjection(camera.viewProjection());
            renderer.beginFrame();
            for (const Entity& entity : scene.entities()) {
                if (entity.mesh == nullptr || entity.texture == nullptr) {
                    continue; // nothing to draw yet — entities may be data-only
                }
                renderer.draw(*entity.mesh, *shader, *entity.texture,
                              interpolate(entity.previousTransform,
                                          entity.transform, alpha));
            }

            if (showDebugDraw) {
                debugDraw.grid(3.0f, 0.5f, {0.35f, 0.35f, 0.40f});
                for (const Entity& entity : scene.entities()) {
                    // Sim-state axes (not interpolated): at demo rates they
                    // trail the blended mesh by under one fixed step — fine
                    // for a debug gizmo, and it shows the raw simulation.
                    debugDraw.axes(entity.transform, 0.5f);
                }
                debugDraw.flush(camera.viewProjection());
            }

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
    } catch (const AssetError& error) {
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
