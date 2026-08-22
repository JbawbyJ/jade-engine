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

    // previous ← current. Call once per render frame, alongside
    // Scene::snapshotPrevious(), BEFORE the fixed-step drain. Also re-arms
    // the once-per-frame mouse-look latch (see fixedUpdate).
    void snapshotPrevious();

    // One fixed simulation step: WASD moves in the horizontal plane relative
    // to yaw only, Space/LeftShift move straight up/down, and holding the
    // right mouse button turns the view with the mouse.
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

    // Mouse look is applied by at most ONE fixed step per render frame.
    // Input::mouseDelta() reports pixels since the last Input::update() —
    // a per-render-frame quantity — so consuming it in every banked step of
    // a catch-up frame would multiply the turn by the step count (a hitch
    // would whip the view around). snapshotPrevious() clears this latch;
    // the first fixedUpdate afterwards sets it and applies the look.
    bool m_lookAppliedThisFrame{false};
};

} // namespace jade
