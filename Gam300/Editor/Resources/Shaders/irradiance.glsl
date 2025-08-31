#version 450 core
layout (location = 0) in vec3 position;

out vec3 worldPosition;

uniform mat4 view;
uniform mat4 proj;

void main() {
    gl_Position = proj * view * vec4(position, 1.0);
    worldPosition = position;
}
==VERTEX==

#version 450 core
out vec4 fragColor;
in vec3 worldPosition;

uniform samplerCube cubeMap;

const float PI = 3.14159265358979323846;

void main() {
    vec3 N = normalize(worldPosition);
    vec3 irradiance = vec3(0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, N);
    up = cross(N, right);

    int nrSamples = 0;
    float sampleDelta = 0.025;

    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            // spherical to cartesian
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

            //to world space
            vec3 samepleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += texture(cubeMap, samepleVec).rgb * cos(theta) * sin(theta);
            ++nrSamples;
        }
    }

    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    fragColor = vec4(irradiance, 1.0);
}
==FRAGMENT==