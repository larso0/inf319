#version 450 core

layout (location = 0) in vec3 fragmentNormal;
layout (location = 1) in vec2 fragmentTextureCoordinate;

layout (location = 0) out vec3 color;

layout (set = 0, binding = 1) uniform LightData {
	vec3 direction;
} lightData;

layout (set = 0, binding = 2) uniform sampler2D texSampler;

void main() {
	float lightIntensity = clamp(dot(lightData.direction, fragmentNormal), 0, 1) * 0.8;
    color = texture(texSampler, fragmentTextureCoordinate).rgb * (0.2 + lightIntensity);;
}
