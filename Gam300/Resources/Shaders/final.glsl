#version 330 core

layout (location = 0) in vec4 quad;
out vec2 uv;

void main() {
    uv = quad.zw;
    gl_Position = vec4(quad.xy, 0.0, 1.0);
}
==VERTEX==

#version 330 core

out vec4 fragColor;
in vec2 uv;
uniform sampler2D map;

void main() {
    fragColor = texture(map, uv);
}
==FRAGMENT==