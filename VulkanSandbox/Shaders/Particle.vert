#version 450
layout (location = 0) in vec3 vertexPosition;
layout (set = 0, binding = 2) uniform Matrices {
	uniform mat4 viewMatrix;
	uniform mat4 projectionMatrix;
};
void main() {
	gl_Position = viewMatrix * vec4(vertexPosition, 1.0);
}
