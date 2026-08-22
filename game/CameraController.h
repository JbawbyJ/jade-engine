#pragma once

// Fly-camera controller: owns the AUTHORITATIVE camera pose (position, yaw,
// pitch) plus the previous-step copies used for render interpolation. The
// Camera it drives is just an output lens — writeToCamera() pushes the
// blended pose into it each render frame, the same snapshot + blend scheme
// the scene uses for entity transforms. Pitch clamping lives here, per the
// Phase 2 freeze (Camera stores what it is given).

#include "math/MathTypes.h"

namespace jade {

class Camera;
class Input;

class CameraController {
public:
    // Seeds the pose (and its previous copies) from the camera's current
    // position/yaw/pitch, so construction never teleports the view.
    explicit CameraController(Camera& camera);

    // Bank this render frame's mouse-look delta (pixels, while the right
    // button is held). Call once per frame right after Input::update():
    // mouseDelta() covers exactly one frame and is overwritten by the next
    // update(), so un-banked motion on a frame that runs zero fixed steps
    // would be lost forever — banking defers it to the next step instead.
    void collectLook(const Input& input);

    // previous ← current. Call once per CONSUMED fixed step, inside the
    // drain, before fixedUpdate — never once per render frame: a per-frame
    // snapshot collapses previous onto current on zero-step frames and turns
    // interpolation into a 60 Hz stair-step (found by the Phase 5 review).
    void snapshotPrevious();

    // One fixed simulation step: WASD moves in the horizontal plane relative
    // to yaw only, Space/LeftShift move straight up/down, and the banked
    // look delta (collectLook) is consumed in full by exactly one step.
    void fixedUpdate(const Input& input, float dt);

    // Writes lerp(previous, current, alpha) into the stored Camera —
    // position lerp, shortest-arc yaw/pitch lerp. Call once per render
    // frame after the drain, before Renderer::setViewProjection.
    void writeToCamera(float alpha) const;

private:
    static constexpr float kPi = 3.14159265358979323846f;
    static constexpr float kMoveSpeed = 2.5f;                    // world units / s
    static constexpr float kLookSpeed = 0.0035f;                 // radians / pixel
    static constexpr float kMaxPitch = 89.0f * kPi / 180.0f;     // gimbal guard

    Camera& m_camera;

    Vec3  m_position{0.0f};
    float m_yaw{0.0f};
    float m_pitch{0.0f};

    Vec3  m_previousPosition{0.0f};
    float m_previousYaw{0.0f};
    float m_previousPitch{0.0f};

    // Pixels of look input banked by collectLook() and not yet consumed.
    // Consumed (and zeroed) by exactly one fixed step, so a catch-up frame
    // cannot multiply the turn and a zero-step frame cannot lose it.
    Vec2 m_pendingLook{0.0f, 0.0f};
};

} // namespace jade
