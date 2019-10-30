#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
uniform mat4 matMVP;
uniform vec4 posCamera;
uniform vec4 dirCamera;

out float flNormalCameraDot;

void main() {
	flNormalCameraDot = max(dot(dirCamera.xyz, aNormal), 0.0f);
    gl_Position = matMVP * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
