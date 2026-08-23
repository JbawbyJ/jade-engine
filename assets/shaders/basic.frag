#version 330 core
in vec3 vNormal;
in vec3 vColor;
in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform vec3 uLightDir; // unit vector from the surface toward the light

out vec4 FragColor;

void main() {
    // Lambert over an ambient floor: unlit faces stay readable, lit faces pop.
    float ndotl = max(dot(normalize(vNormal), normalize(uLightDir)), 0.0);
    float lit = 0.15 + 0.85 * ndotl;
    FragColor = vec4(vColor * lit, 1.0) * texture(uTexture, vTexCoord);
}
