#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
    vec4 gl_Position;
};

layout (location = 0) out vec3 fragmentNormal;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;

layout (location = 0) uniform Matrices {
	mat4 mvp;
	mat4 normal;
} matrices;

void main() {
    gl_Position = matrices.mvp * vec4(vertexPosition, 1.0);
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
    gl_Position.y = -gl_Position.y;
    fragmentNormal = normalize(matrices.normal * vec4(vertexNormal, 0)).xyz;
}
