#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 fragmentNormal;
layout (location = 1) in vec3 fragmentColor;

layout (location = 0) out vec3 color;

layout (set = 0, binding = 1) uniform LightData {
	vec3 direction;
} lightData;

void main() {
	float lightIntensity = clamp(dot(lightData.direction, fragmentNormal), 0, 1) * 0.8;
    color = fragmentColor * (0.2 + lightIntensity);;
}
