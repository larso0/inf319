#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec3 fragmentNormal;
layout (location = 1) out vec3 fragmentColor;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;

layout (set = 0, binding = 0) uniform EntityData {
	mat4 mvp;
	mat4 normal;
	vec4 color;
} data;

void main() {
    gl_Position = data.mvp * vec4(vertexPosition, 1.0);
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
    gl_Position.y = -gl_Position.y;
    fragmentNormal = normalize(data.normal * vec4(vertexNormal, 0)).xyz;
    fragmentColor = data.color.rgb;
}
