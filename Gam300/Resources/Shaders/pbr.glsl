#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out Vertex {
    vec3 position;
    vec3 normal;
} vertex;

uniform mat4 modelMat;
uniform mat4 frustumMat; //projection * view

void main() {
    vertex.normal = mat3(modelMat) * normal;
    vec4 worldPos = modelMat * vec4(position, 1.0);
    vertex.position = worldPos.xyz;
    gl_Position = frustumMat * worldPos;
}
==VERTEX==

#version 450 core
layout (location = 0) out vec4 fragColor;

in Vertex {
    vec3 position;
    vec3 normal;
} vertex;

struct Material {
    vec3 albedo;
    float roughness;
    float metallic;
};
uniform Material material;
uniform vec3 viewPos;

const float PI = 3.14159265358979323846;
const int MAX_LIGHTS = 10; //this number has to be redefined here due to glsl constrains

struct PointLight {
    vec3 position;
    vec3 radiance;
    float intensity;
};
uniform PointLight pointLights[MAX_LIGHTS];
uniform int noPointLight;

struct DirectionalLight {
    vec3 radiance;
    vec3 dir;
    float intensity;
};
uniform DirectionalLight dirLights[MAX_LIGHTS];
uniform int noDirLight;

struct SpotLight {
    vec3 dir;
    vec3 position;
    vec3 radiance;
    float intensity;
    float fallOff;
    float cutOff;
};
uniform SpotLight spotLights[MAX_LIGHTS];
uniform int noSpotLight;

//this effect influences the appearance of surfaces
// for example, higher reflectivity at grazing angles than dielectrics
// f0 stands for the base fresnel distributivity
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
vec3 ComputeDirLights(vec3 N, vec3 V, vec3 f0);
vec3 ComputeSpotLights(vec3 N, vec3 V, vec3 f0);

void main() {
    
    
    vec3 N = normalize(vertex.normal);
    vec3 V = normalize(viewPos - vertex.position);
    vec3 f0 = mix(vec3(0.04), material.albedo, material.metallic);
    vec3 color = ComputePointLights(N, V, f0) + ComputeDirLights(N, V, f0) + ComputeSpotLights(N, V, f0);
    //color += 0.03 * material.albedo; // Ambient
    
    fragColor = vec4(color, 1.0);
    //fragColor = vec4(normalize(vertex.normal) * 0.5 + 0.5, 1.0); //normal map colors
}

vec3 FresnelSchlick(float cosTheta, vec3 f0) {
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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

        vec3 L = normalize(pointLights[i].position - vertex.position);
        vec3 H = normalize(L + V);
        float nDotL = max(dot(N, L), 0.0);
        float nDotV = max(dot(N, V), 0.0);

        //Cook-Torrance (BRDF)
        float NDF = DistributionGGX(N, H, material.roughness);
        vec3 FS = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), f0);
        float GS = GeometrySmithGGX(nDotV, nDotL, material.roughness);

        vec3 diffuse = (vec3(1.0) - FS) * (1.0 - material.metallic) * material.albedo / PI;
        vec3 specular = (NDF * GS * FS) / max(4.0 * nDotV * nDotL, 0.0001);
        float dist = length(pointLights[i].position - vertex.position);
        float attenuation = pointLights[i].intensity / (dist * dist);

        result += (diffuse + specular) * pointLights[i].radiance * attenuation * nDotL;
    }

    return result;
}
vec3 ComputeDirLights(vec3 N, vec3 V, vec3 f0) {
    vec3 result = vec3(0.0);

    for (int i = 0; i < noDirLight; ++i) {
        //break condition
        if (i >= MAX_LIGHTS) {
            break;
        }

        vec3 L = -normalize(dirLights[i].dir);
        float nDotL = max(dot(N, L), 0.0);
        float nDotV = max(dot(N, V), 0.0);
        vec3 H = normalize(L + V);

        //BRDF
        float NDF = DistributionGGX(N, H, material.roughness);
        vec3 FS = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), f0);
        float GS = GeometrySmithGGX(nDotV, nDotL, material.roughness);

        vec3 diffuse = (vec3(1.0) - FS) * (1.0 - material.metallic) * material.albedo / PI;
        vec3 specular = (NDF * GS * FS) / max(4.0 * nDotV * nDotL, 0.0001);

        result += (diffuse + specular) * dirLights[i].radiance * dirLights[i].intensity * nDotL;
    }

    return result;
}
vec3 ComputeSpotLights(vec3 N, vec3 V, vec3 f0) {
    vec3 result = vec3(0.0);

    for (int i = 0; i < noSpotLight; ++i) {
        //break condition
        if (i >= MAX_LIGHTS) {
            break;
        }

        vec3 L = normalize(spotLights[i].position - vertex.position);
        float nDotL = max(dot(N, L), 0.0);
        float nDotV = max(dot(N, V), 0.0);
        vec3 H = normalize(L + V);

        //BRDF
        float NDF = DistributionGGX(N, H, material.roughness);
        vec3 FS = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), f0);
        float GS = GeometrySmithGGX(nDotV, nDotL, material.roughness);

        vec3 diffuse = (vec3(1.0) - FS) * (1.0 - material.metallic) ;
        vec3 specular = (NDF * GS * FS) / max(4.0 * nDotV * nDotL, 0.0001);

        //compute spot
        float theta = dot(L, normalize(-spotLights[i].dir));
        float epsilon = max(spotLights[i].fallOff - spotLights[i].cutOff, 0.0001);
        float spotFactor = clamp((theta - spotLights[i].cutOff) / epsilon, 0.0, 1.0);

        float dist = length(spotLights[i].position - vertex.position);
        float attenuation = spotLights[i].intensity / (dist * dist);

        result += (diffuse * material.albedo / PI + specular) * 
                    spotLights[i].radiance * 
                    spotLights[i].intensity * 
                    attenuation * 
                    nDotL
                    * spotFactor;
    }

    return result;
}
==FRAGMENT==