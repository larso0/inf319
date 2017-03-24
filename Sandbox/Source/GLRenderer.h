#ifndef GLRENDERER_H
#define GLRENDERER_H

#include <Engine/Renderer.h>
#include <unordered_map>
#include <memory>
#include "GLPerMesh.h"
#include "GLWindow.h"

class GLRenderer : public Engine::Renderer {
public:
	GLRenderer(GLWindow& window);
	~GLRenderer();

	void render(const Engine::Camera& camera,
		const std::vector<Engine::Entity>& entities);

private:
	GLWindow& window;
	GLuint drawProgram;
	GLint vertexPosition, vertexNormal, vertexTextureCoordinate;
	GLint worldViewProjectionMatrixUniform, normalMatrixUniform, entityColorUniform;
	std::unordered_map<const Engine::Mesh*, std::shared_ptr<GLPerMesh>> meshCache;
};

#endif
