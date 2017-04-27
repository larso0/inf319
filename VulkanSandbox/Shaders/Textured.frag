#version 450 core

layout (location = 0) in vec3 fragmentNormal;
layout (location = 1) in vec2 fragmentTextureCoordinate;

layout (location = 0) out vec3 color;

layout (set = 0, binding = 1) uniform LightData {
	vec4 direction;
    vec4 color;
} lightData;

layout (set = 0, binding = 2) uniform sampler2D texSampler;

void main() {
    vec3 light = clamp(dot(lightData.direction.xyz, fragmentNormal), 0.0, 1.0) * lightData.color.rgb * 0.85;
    vec3 texColor = texture(texSampler, fragmentTextureCoordinate).rgb;
    color = texColor * 0.15 + texColor * light;
}
