#include "GLRenderer.h"
#include <stdexcept>
#include <sstream>

using namespace std;
using namespace Engine;

const char* vertexShaderSource =
	"#version 450\n"
	"in vec3 vertexPosition;\n"
	"in vec3 vertexNormal;\n"
	"out vec3 fragmentNormal;\n"
	"uniform mat4 worldViewProjectionMatrix;\n"
	"uniform mat3 normalMatrix;\n"
	"void main() {\n"
	"	fragmentNormal = normalize(normalMatrix * vertexNormal);\n"
	"	gl_Position = worldViewProjectionMatrix * vec4(vertexPosition, 1);\n"
	"}\n";

const char* fragmentShaderSource =
	"#version 450\n"
	"in vec3 fragmentNormal;\n"
	"out vec3 color;\n"
	"uniform vec3 entityColor;\n"
	"uniform vec3 lightDirection;\n"
	"uniform vec3 lightColor;\n"
	"void main() {\n"
	"	vec3 light = clamp(dot(lightDirection, fragmentNormal), 0, 1) * lightColor;\n"
	"	color = entityColor * 0.1 + entityColor * light;\n"
	"}\n";

static GLuint createShader(GLenum type, const char* source) {
	GLuint handle = glCreateShader(type);
	glShaderSource(handle, 1, &source, nullptr);

	glCompileShader(handle);
	GLint status = 0;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint len = 0;
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &len);
		char* log = new char[len];
		glGetShaderInfoLog(handle, len, &len, log);
		stringstream ss;
		ss << "Error when compiling shader: " << log;
		delete[] log;
		glDeleteShader(handle);
		throw runtime_error(ss.str());
	}
	return handle;
}

static GLuint createDrawProgram() {
	GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER,
		fragmentShaderSource);
	GLuint handle = glCreateProgram();

	glAttachShader(handle, vertexShader);
	if (glGetError() != GL_NO_ERROR) {
		throw runtime_error("Unable to attach vertex shader.");
	}

	glAttachShader(handle, fragmentShader);
	if (glGetError() != GL_NO_ERROR) {
		glDeleteShader(vertexShader);
		throw runtime_error("Unable to attach fragment shader.");
	}

	glLinkProgram(handle);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	GLint status = 0;
	glGetProgramiv(handle, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint len = 0;
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &len);
		char* log = new char[len];
		glGetProgramInfoLog(handle, len, &len, log);
		stringstream ss;
		ss << "Error when linking program: " << log;
		delete[] log;
		glDeleteProgram(handle);
		throw runtime_error(ss.str());
	}

	return handle;
}

GLRenderer::GLRenderer(GLWindow& window) :
Renderer(),
window(window)
{
	drawProgram = createDrawProgram();
	vertexPosition = glGetAttribLocation(drawProgram, "vertexPosition");
	vertexNormal = glGetAttribLocation(drawProgram, "vertexNormal");
	vertexTextureCoordinate = glGetAttribLocation(drawProgram, "vertexTextureCoordinate");
	worldViewProjectionMatrixUniform = glGetUniformLocation(drawProgram, "worldViewProjectionMatrix");
	normalMatrixUniform = glGetUniformLocation(drawProgram, "normalMatrix");
	entityColorUniform = glGetUniformLocation(drawProgram, "entityColor");
	lightDirectionUniform = glGetUniformLocation(drawProgram, "lightDirection");
	lightColorUniform = glGetUniformLocation(drawProgram, "lightColor");
	glUseProgram(drawProgram);
}

GLRenderer::~GLRenderer() {
	glDeleteProgram(drawProgram);
}

void GLRenderer::render() {
#ifndef NDEBUG
	if (camera == nullptr) {
		throw runtime_error("No camera set.");
	}
	if (lightSources.size() < 1) {
		throw runtime_error("No light source.");
	}
#endif
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (const Entity* e : entities) {
		const Mesh* mesh = e->getMesh();
		auto result = meshCache.find(mesh);
		if (result == meshCache.end()) {
			shared_ptr<GLPerMesh> perMesh = make_shared<GLPerMesh>(mesh,
				vertexPosition, vertexNormal, vertexTextureCoordinate);
			meshCache[mesh] = perMesh;
		}

		glm::mat4 worldMatrix = e->getNode()->getWorldMatrix()
			* e->getScaleMatrix();
		glm::mat4 worldViewProjectionMatrix = camera->getProjectionMatrix()
			* camera->getViewMatrix() * worldMatrix;
		glm::mat3 normalMatrix =
			glm::mat3(glm::transpose(glm::inverse(worldMatrix)));

		glUniformMatrix4fv(worldViewProjectionMatrixUniform, 1, GL_FALSE,
			glm::value_ptr(worldViewProjectionMatrix));
		glUniformMatrix3fv(normalMatrixUniform, 1, GL_FALSE,
			glm::value_ptr(normalMatrix));
		glUniform3fv(entityColorUniform, 1,
			glm::value_ptr(e->getMaterial()->getColor()));
		glUniform3fv(lightDirectionUniform, 1,
			glm::value_ptr(lightSources[0]->getDirection()));
		glUniform3fv(lightColorUniform, 1,
			glm::value_ptr(lightSources[0]->getColor()));

		shared_ptr<GLPerMesh> perMesh = meshCache[mesh];
		perMesh->bind();
		perMesh->draw();
	}
	window.present();
}
