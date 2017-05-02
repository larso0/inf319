#include "ParticleSystem.h"

#include <stdexcept>

using namespace Engine;
using namespace std;
using glm::vec3;

ParticleSystem::ParticleSystem(Node* emitter, unsigned maxParticles) :
emitter(emitter),
maxParticles(maxParticles),
particleCount(0),
front(0),
particleBuffer(bp::buffer_object(GL_DYNAMIC_DRAW))
{
	particleBuffer.buffer_data(nullptr, sizeof(Particle)*maxParticles);

	/*bp::shader cshader(
		GL_COMPUTE_SHADER,
		"#version 450\n"
		"layout(local_size_x = 32) in;\n"
		"struct Particle { vec3 position; vec3 velocity; };\n"
		"layout (binding = 0) buffer ParticleBuffer { Particle particles[]; };\n"
		"\n"
		"\n"
		"\n"
	);

	particleComputeProgram.attach(cshader);*/

	bp::shader vshader(
		GL_VERTEX_SHADER,
		"#version 450\n"
		"in vec3 vertexPosition;\n"
		"uniform mat4 viewMatrix;\n"
		"void main() {\n"
		"	gl_Position = viewMatrix * vec4(vertexPosition, 1.0);\n"
		"}\n"
	);

	bp::shader gshader(
		GL_GEOMETRY_SHADER,
		"#version 450\n"
		"layout (points) in;\n"
		"layout (triangle_strip, max_vertices = 4) out;\n"
		"uniform mat4 projectionMatrix;\n"
		"void main() {\n"
		"	vec4 pos = gl_in[0].gl_Position;\n"
		"	gl_Position = pos;\n"
		"	gl_Position.x -= 0.5;\n"
		"	gl_Position.y += 0.5;\n"
		"	gl_Position = projectionMatrix * gl_Position;\n"
		"	EmitVertex();\n"
		"	gl_Position = pos;\n"
		"	gl_Position.x -= 0.5;\n"
		"	gl_Position.y -= 0.5;\n"
		"	gl_Position = projectionMatrix * gl_Position;\n"
		"	EmitVertex();\n"
		"	gl_Position = pos;\n"
		"	gl_Position.x += 0.5;\n"
		"	gl_Position.y += 0.5;\n"
		"	gl_Position = projectionMatrix * gl_Position;\n"
		"	EmitVertex();\n"
		"	gl_Position = pos;\n"
		"	gl_Position.x += 0.5;\n"
		"	gl_Position.y -= 0.5;\n"
		"	gl_Position = projectionMatrix * gl_Position;\n"
		"	EmitVertex();\n"
		"	EndPrimitive();\n"
		"}\n"
	);

	bp::shader fshader(
		GL_FRAGMENT_SHADER,
		"#version 450\n"
		"out vec3 color;\n"
		"void main() {\n"
		"	color = vec3(1.0, 0.0, 0.0);\n"
		"}\n"
	);

	particleDrawProgram.attach(vshader);
	particleDrawProgram.attach(gshader);
	particleDrawProgram.attach(fshader);
	glPointSize(10.f);

	int posAttribute = particleDrawProgram.attribute("vertexPosition");
	viewMatrixUniform = particleDrawProgram.uniform("viewMatrix");
	projectionMatrixUniform = particleDrawProgram.uniform("projectionMatrix");

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	particleBuffer.bind(GL_ARRAY_BUFFER);
	glEnableVertexAttribArray(posAttribute);
	glVertexAttribPointer(posAttribute, 3, GL_FLOAT, false, sizeof(Particle), 0);

	//Temporary particles for testing
	Particle* particles = (Particle*)
		glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(Particle)*4, GL_MAP_WRITE_BIT);
	particles[0] = { vec3(0.f, 0.f, 0.f), vec3() };
	particles[1] = { vec3(-1.f, -2.f, -2.f), vec3() };
	particles[2] = { vec3(-2.f, 1.f, -1.f), vec3() };
	particles[3] = { vec3(-2.f, 2.f, -2.f), vec3() };
	glUnmapBuffer(GL_ARRAY_BUFFER);
	particleCount = 4;
	front = 4;
}

ParticleSystem::~ParticleSystem() {
	glDeleteVertexArrays(1, &vao);
}

void ParticleSystem::draw(const Camera& camera) {
	particleDrawProgram.use();
	glUniformMatrix4fv(viewMatrixUniform, 1, false, glm::value_ptr(camera.getViewMatrix()));
	glUniformMatrix4fv(projectionMatrixUniform, 1, false, glm::value_ptr(camera.getProjectionMatrix()));
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, particleCount);
}
