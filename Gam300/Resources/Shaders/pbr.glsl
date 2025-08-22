#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (locaiton = 2) in vec2 uv;

uniform mat4 modelMat;
uniform mat4 proj;
uniform mat4 view;

void main() {
    gl_Position = proj * view * modelMat * vec4(position, 1.0);
}
==VERTEX==

#version 450 core
layout (location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(0.6, 0.5, 0.7, 1.0);
}
==FRAGMENT==