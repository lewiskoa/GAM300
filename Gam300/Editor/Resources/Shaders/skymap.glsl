#version 450 core
layout (location = 0) in vec3 position;

out vec3 worldPosition;

uniform mat4 proj;
uniform mat4 view;

void main() {
    gl_Position = proj * view * vec4(position, 1.0);
    worldPosition = normalize(position);
}
==VERTEX==

#version 450 core
layout (location = 0) out vec4 fragColor;

in vec3 worldPosition;

uniform sampler2D map;

//converts 3D spherical coord to 2D texture coord
//equirectangular mapping
vec2 GetSphericalUV(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183); //scaling
    uv += 0.5;                  //offset
    return uv;
}

void main() {
    fragColor = vec4(texture(map, GetSphericalUV(worldPosition)).rgb, 1.0);
}
==FRAGMENT==