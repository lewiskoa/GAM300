#version 450 core
layout (location = 0) in vec3 position;

out vec3 worldPosition;

uniform mat4 modelMat;
uniform mat4 proj;
uniform mat4 view;

void main() {
    //render skybox around the scene instead of somewhere else
    vec4 pos = proj * mat4(mat3(view)) * modelMat * vec4(position, 1.0);
    gl_Position = pos.xyww;
    worldPosition = position;
}
==VERTEX==

#version 450 core
layout (location = 0) out vec4 fragColor;

in vec3 worldPosition;

uniform samplerCube map;

void main() {
    fragColor = vec4(texture(map, worldPosition).rgb, 1.0);
}
==FRAGMENT==