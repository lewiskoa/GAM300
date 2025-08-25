#version 450 core

layout(location = 0) in vec2 pos;

void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
}
==VERTEX==

#version 450 core

out vec4 FragColor;
uniform vec4 color;
void main() {
    FragColor = color;
}
==FRAGMENT==