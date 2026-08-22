#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

uniform mat4 uViewProj;
uniform mat4 uModel;

out vec3 vNormal;
out vec3 vColor;
out vec2 vTexCoord;

void main() {
    // Normal matrix keeps normals perpendicular under non-uniform scale.
    // TODO(jade): hoist to a uniform when per-vertex inverse() shows up in a profile.
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vColor = aColor;
    vTexCoord = aTexCoord;
    gl_Position = uViewProj * uModel * vec4(aPosition, 1.0);
}
