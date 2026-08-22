#include "math/Transform.h"

#include <glm/gtc/matrix_transform.hpp>

namespace jade {

Mat4 Transform::matrix() const {
    // T · Ry · Rx · Rz · S — glm::rotate/translate/scale each post-multiply,
    // so applying them in this order builds exactly that product.
    Mat4 m{1.0f};
    m = glm::translate(m, position);
    m = glm::rotate(m, rotationEuler.y, Vec3{0.0f, 1.0f, 0.0f});
    m = glm::rotate(m, rotationEuler.x, Vec3{1.0f, 0.0f, 0.0f});
    m = glm::rotate(m, rotationEuler.z, Vec3{0.0f, 0.0f, 1.0f});
    m = glm::scale(m, scale);
    return m;
}

} // namespace jade
