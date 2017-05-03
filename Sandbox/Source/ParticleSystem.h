#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <Engine/Node.h>
#include <glad/glad.h>
#include <bp/buffer.h>
#include <bp/program.h>
#include <Engine/Camera.h>

class ParticleSystem {
public:
	ParticleSystem(Engine::Node* emitter, unsigned maxParticles = 1024);
	~ParticleSystem();

	void compute(float deltaTime);
	void draw(const Engine::Camera& camera);
	void emit(float speed, const glm::vec3& direction);

private:
	Engine::Node* emitter;
	unsigned maxParticles;
	unsigned particleCount;
	unsigned front;

	struct Particle {
		glm::vec3 position;
		float pad0;
		glm::vec3 velocity;
		float pad1;
	};

	bp::buffer_object particleBuffer;
	bp::program particleComputeProgram;
	bp::program particleDrawProgram;
	GLuint vao;

	GLint viewMatrixUniform, projectionMatrixUniform, deltaTimeUniform;
};

#endif
