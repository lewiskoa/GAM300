#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out Vertex {
    vec3 vposition;
    vec3 vnormal;
} vertex;

uniform mat4 modelMat;
uniform mat4 frustumMat; //projection * view

void main() {
    vertex.vnormal = mat3(modelMat) * normal;
    vertex.vposition = vec3(modelMat * vec4(position, 1.0));
    gl_Position = frustumMat * modelMat * vec4(position, 1.0);
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
};
uniform Material material;
uniform vec3 viewPos;

const float PI = 3.141592653589793;
const int MAX_LIGHTS = 10; //this number has to be redefined here due to glsl constrains

struct PointLight {
    vec3 radiance;
    float intensity;
    vec3 position;
};

uniform PointLight pointLights[MAX_LIGHTS];
uniform int noPointLight;

//this effect influences the appearance of surfaces
// for example, higher reflectivity at grazing angles than dielectrics
vec3 FresnelSchlick(float cosTheta, vec3 f0);
//approximate Fresnel factor for the microfacet distribution model
// for the material's reflectivity based on viewing angle
float GeometrySchlickGGX(float nDotV, float roughness);
//computes visibility term that accounts for self-shadowing 
// and masking effects due to microfacet distribution
float GeometrySmithGGX(float nDotV, float nDotL, float roughness);
//Trowbridge-Reitz calculates the probability distribution 
// of surface normals based on surface roughness
float DistributionGGX(vec3 N, vec3 H, float roughness);
vec3 ComputePointLights(vec3 N, vec3 V, vec3 f0);

void main() {
    //diffuse
    vec3 N = normalize(vertex.vnormal);
    vec3 V = normalize(viewPos - vertex.vposition);
    vec3 fresnel = mix(vec3(0.04), material.albedo, material.metallic);
    vec3 color = ComputePointLights(N, V, fresnel);

    fragColor = vec4(color, 1.0);
}

vec3 FresnelSchlick(float cosTheta, vec3 f0) {
    return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}
float GeometrySchlickGGX(float nDotV, float roughness) {
    float r = roughness + 1.0;
    float k = r * r / 8.0;
    float denom = nDotV * (1.0 - k) + k;
    return nDotV / denom;
}
float GeometrySmithGGX(float nDotV, float nDotL, float roughness) {
    float ggx1 = GeometrySchlickGGX(nDotV, roughness);
    float ggx2 = GeometrySchlickGGX(nDotL, roughness);
    return ggx1 * ggx2;
}
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float aSq = (roughness * roughness) * (roughness * roughness);
    float nDotH = max(dot(N, H), 0.0);
    float nDotHSq = nDotH * nDotH;
    float denom = nDotHSq * (aSq - 1.0) + 1.0;
    return aSq / (PI * denom * denom);
}
vec3 ComputePointLights(vec3 N, vec3 V, vec3 f0) {
    vec3 result = vec3(0.0); 

    for (int i = 0; i < noPointLight; ++i) {
        //break condition
        if (i >= MAX_LIGHTS) {
            break;
        }

        vec3 L = normalize(pointLights[i].position - vertex.vposition);
        vec3 H = normalize(L + V);
        float nDotL = max(dot(N, L), 0.0);
        float nDotV = max(dot(N, V), 0.0);

        //Cook-Torrance (BRDF)
        float NDF = DistributionGGX(N, H, material.roughness);
        vec3 FS = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), f0);
        float GS = GeometrySmithGGX(nDotV, nDotL, material.roughness);

        vec3 diffuse = (vec3(1.0) - FS) * (1.0 - material.metallic);
        vec3 specular = (NDF * GS * FS) / max(4.0 * nDotV * nDotL, 0.0001);
        float dist = length(pointLights[i].position - vertex.vposition);
        float attenuation = pointLights[i].intensity / (dist * dist);

        result += (diffuse * material.albedo / PI + specular) * 
                    pointLights[i].radiance * attenuation * nDotL;
    }

    return result;
}
==FRAGMENT==