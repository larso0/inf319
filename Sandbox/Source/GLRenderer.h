#ifndef GLRENDERER_H
#define GLRENDERER_H

#include <Engine/Renderer.h>
#include <unordered_map>
#include <memory>
#include "GLPerMesh.h"
#include "GLWindow.h"
#include "ParticleSystem.h"

class GLRenderer : public Engine::Renderer {
public:
	GLRenderer(GLWindow& window);
	~GLRenderer();

	void render() override;

	void setTextureAtlas(const Engine::TextureAtlas* atlas) override;
	void setParticleSystem(ParticleSystem* ps) {
		particleSystem = ps;
	}

private:
	GLWindow& window;
	GLuint drawProgram, texturedDrawProgram;
	GLint vertexPosition, vertexNormal;
	GLint texturedVertexPosition, texturedVertexNormal, texturedVertexTextureCoordinate;
	GLint worldViewProjectionMatrixUniform,
		  normalMatrixUniform,
		  entityColorUniform,
		  lightDirectionUniform,
		  lightColorUniform;
	GLint texturedWorldViewProjectionMatrixUniform,
		  texturedNormalMatrixUniform,
		  texturedLightDirectionUniform,
		  texturedLightColorUniform,
		  textureRegionUniform,
		  textureScaleUniform;
	std::unordered_map<const Engine::Mesh*, std::shared_ptr<GLPerMesh>> meshCache;
	GLuint texture;
	bool haveTexture;

	ParticleSystem* particleSystem;
	double currentTime;
};

#endif
