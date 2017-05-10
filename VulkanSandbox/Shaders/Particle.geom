#version 450
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;
layout (set = 0, binding = 2) uniform Matrices {
	uniform mat4 viewMatrix;
	uniform mat4 projectionMatrix;
};
void main() {
	float s = 0.1f;
	vec4 pos = gl_in[0].gl_Position;
	gl_Position = pos;
	gl_Position.x -= s;
	gl_Position.y += s;
	gl_Position = projectionMatrix * gl_Position;
	EmitVertex();
	gl_Position = pos;
	gl_Position.x -= s;
	gl_Position.y -= s;
	gl_Position = projectionMatrix * gl_Position;
	EmitVertex();
	gl_Position = pos;
	gl_Position.x += s;
	gl_Position.y += s;
	gl_Position = projectionMatrix * gl_Position;
	EmitVertex();
	gl_Position = pos;
	gl_Position.x += s;
	gl_Position.y -= s;
	gl_Position = projectionMatrix * gl_Position;
	EmitVertex();
	EndPrimitive();
}
