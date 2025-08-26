#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out Vertex {
    vec3 vposition;
    vec3 vnormal;
} vertex;

uniform mat4 modelMat;
uniform mat4 proj;
uniform mat4 view;

void main() {
    vertex.vnormal = mat3(modelMat) * normal;
    vertex.vposition = vec3(modelMat * vec4(position, 1.0));
    gl_Position = proj * view * modelMat * vec4(position, 1.0);
}
==VERTEX==

#version 450 core
layout (location = 0) out vec4 fragColor;

in Vertex {
    vec3 vposition;
    vec3 vnormal;
} vertex;

struct Material {
    vec3 albedo;
    float roughness;
    float metallic;
}
uniform Material material;

void main() {
    vec3 lightPos = vec3(-0.5, 0.0, 1.0);
    vec3 lightRadiance = vec3(1.0, 1.0, 1.0);
    vec3 viewPos = vec3(0.0, 0.0, 2.0);

    //diffuse
    vec3 N = normalize(vertex.vnormal);
    vec3 lightDir = normalize(lightPos - vertex.vposition);
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * lightRadiance * material.albedo;

    //Dpecular (cook-torrance)
    vec3 viewDir = normalize(viewPos - vertex.vposition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(N, halfwayDir), 0.0);
    float roughnessSq = material.roughness * material.roughness;
    float denom = (NdotH * NdotH) * (roughnessSq - 1.0) + 1.0;
    float D = roughnessSq / (3.141 * denom * denom);
    vec3 specular = lightRadiance * (material.metallic * D);

    vec3 color = (diffuse + specular);

    fragColor = vec4(color, 1.0);
}
==FRAGMENT==