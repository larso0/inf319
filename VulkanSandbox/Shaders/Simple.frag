#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 fragmentNormal;

layout(location = 0) out vec4 fragmentColor;

void main() {
    fragmentColor = vec4(fragmentNormal, 1.0);
}
