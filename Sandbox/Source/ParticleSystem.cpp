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

	bp::shader cshader(
		GL_COMPUTE_SHADER,
		"#version 450\n"
		"layout (local_size_x = 1) in;\n"
		"struct Particle { vec3 position; vec3 velocity; };\n"
		"layout (binding = 0) buffer ParticleBuffer { Particle particles[]; };\n"
		"uniform float deltaTime;\n"
		"void main() {\n"
		"	uint i = gl_GlobalInvocationID.x;\n"
		"	particles[i].position += deltaTime * particles[i].velocity;\n"
		"}\n"
	);

	particleComputeProgram.attach(cshader);
	deltaTimeUniform = particleComputeProgram.uniform("deltaTime");

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
		"	float s = 0.1f;\n"
		"	vec4 pos = gl_in[0].gl_Position;\n"
		"	gl_Position = pos;\n"
		"	gl_Position.x -= s;\n"
		"	gl_Position.y += s;\n"
		"	gl_Position = projectionMatrix * gl_Position;\n"
		"	EmitVertex();\n"
		"	gl_Position = pos;\n"
		"	gl_Position.x -= s;\n"
		"	gl_Position.y -= s;\n"
		"	gl_Position = projectionMatrix * gl_Position;\n"
		"	EmitVertex();\n"
		"	gl_Position = pos;\n"
		"	gl_Position.x += s;\n"
		"	gl_Position.y += s;\n"
		"	gl_Position = projectionMatrix * gl_Position;\n"
		"	EmitVertex();\n"
		"	gl_Position = pos;\n"
		"	gl_Position.x += s;\n"
		"	gl_Position.y -= s;\n"
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
}

ParticleSystem::~ParticleSystem() {
	glDeleteVertexArrays(1, &vao);
}

void ParticleSystem::compute(float deltaTime) {
	particleComputeProgram.use();
	glUniform1f(deltaTimeUniform, deltaTime);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBuffer.handle());
	glDispatchCompute(particleCount, 1, 1);
}

void ParticleSystem::draw(const Camera& camera) {
	particleDrawProgram.use();
	glUniformMatrix4fv(viewMatrixUniform, 1, false, glm::value_ptr(camera.getViewMatrix()));
	glUniformMatrix4fv(projectionMatrixUniform, 1, false, glm::value_ptr(camera.getProjectionMatrix()));
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, particleCount);
}

void ParticleSystem::emit(float speed, const glm::vec3& direction) {
	glm::vec3 velocity =
		quatTransform(emitter->getOrientation(), direction) * speed;
	particleBuffer.bind(GL_ARRAY_BUFFER);
	Particle* particle = (Particle*)
		glMapBufferRange(GL_ARRAY_BUFFER, front * sizeof(Particle),
			sizeof(Particle), GL_MAP_WRITE_BIT);
	particle->position = emitter->getPosition();
	particle->velocity = velocity;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	if (particleCount < maxParticles) {
		particleCount++;
		front++;
	} else {
		front = (front + 1) % maxParticles;
	}
}
