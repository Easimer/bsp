#version 330 core
out vec4 FragColor;

in float flNormalCameraDot;

void main() {
    FragColor = vec4(0.5f + 0.5f * flNormalCameraDot, 0.5f, 0.2f, 1.0f);
}
