#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 fragmentNormal;
layout (location = 1) in vec3 fragmentColor;

layout (location = 0) out vec4 color;

void main() {
    color = vec4(fragmentColor * 0.8 + fragmentNormal * 0.2 , 1.0);
}
