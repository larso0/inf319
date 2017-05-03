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
	"	vec3 light = clamp(dot(lightDirection, fragmentNormal), 0.0, 1.0) * lightColor * 0.85;\n"
	"	color = entityColor * 0.15 + entityColor * light;\n"
	"}\n";

const char* texturedVertexShaderSource =
	"#version 450\n"
	"in vec3 vertexPosition;\n"
	"in vec3 vertexNormal;\n"
	"in vec2 vertexTextureCoordinate;\n"
	"out vec3 fragmentNormal;\n"
	"out vec2 fragmentTextureCoordinate;\n"
	"uniform mat4 worldViewProjectionMatrix;\n"
	"uniform mat3 normalMatrix;\n"
	"uniform vec4 textureRegion;\n"
	"uniform vec2 textureScale;\n"
	"void main() {\n"
	"	fragmentNormal = normalize(normalMatrix * vertexNormal);\n"
	"	fragmentTextureCoordinate = fract(vertexTextureCoordinate * textureScale);\n"
	"	fragmentTextureCoordinate *= vec2(textureRegion.z, textureRegion.w);\n"
	"	fragmentTextureCoordinate += textureRegion.xy;\n"
	"	gl_Position = worldViewProjectionMatrix * vec4(vertexPosition, 1);\n"
	"}\n";

const char* texturedFragmentShaderSource =
	"#version 450\n"
	"in vec3 fragmentNormal;\n"
	"in vec2 fragmentTextureCoordinate;\n"
	"out vec3 color;\n"
	"uniform vec3 lightDirection;\n"
	"uniform vec3 lightColor;\n"
	"uniform sampler2D texSampler;\n"
	"void main() {\n"
	"	vec3 light = clamp(dot(lightDirection, fragmentNormal), 0.0, 1.0) * lightColor * 0.85;\n"
	"	vec3 texColor = texture(texSampler, fragmentTextureCoordinate).rgb;\n"
	"	color = texColor * 0.15 + texColor * light;\n"
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

static GLuint createDrawProgram(const char* vshader, const char* fshader) {
	GLuint vertexShader = createShader(GL_VERTEX_SHADER, vshader);
	GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fshader);
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

static GLuint createTexture(const Texture* tex) {
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);

	GLint internalFormat;
	GLenum format;
	switch(tex->getFormat()) {
	case Texture::Format::Grey:
		internalFormat = GL_RED;
		format = GL_RED;
		break;
	case Texture::Format::GreyAlpha:
		internalFormat = GL_RG;
		format = GL_RG;
		break;
	case Texture::Format::RGBA:
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		break;
	}

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		internalFormat,
		tex->getWidth(),
		tex->getHeight(),
		0,
		format,
		GL_UNSIGNED_BYTE,
		tex->getPixelData()
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	return handle;
}

GLRenderer::GLRenderer(GLWindow& window) :
Renderer(),
window(window),
texture(0),
haveTexture(false),
particleSystem(nullptr)
{
	drawProgram = createDrawProgram(vertexShaderSource, fragmentShaderSource);
	texturedDrawProgram = createDrawProgram(texturedVertexShaderSource, texturedFragmentShaderSource);

	vertexPosition = glGetAttribLocation(drawProgram, "vertexPosition");
	vertexNormal = glGetAttribLocation(drawProgram, "vertexNormal");
	worldViewProjectionMatrixUniform = glGetUniformLocation(drawProgram, "worldViewProjectionMatrix");
	normalMatrixUniform = glGetUniformLocation(drawProgram, "normalMatrix");
	entityColorUniform = glGetUniformLocation(drawProgram, "entityColor");
	lightDirectionUniform = glGetUniformLocation(drawProgram, "lightDirection");
	lightColorUniform = glGetUniformLocation(drawProgram, "lightColor");

	texturedVertexPosition = glGetAttribLocation(texturedDrawProgram, "vertexPosition");
	texturedVertexNormal = glGetAttribLocation(texturedDrawProgram, "vertexNormal");
	texturedVertexTextureCoordinate = glGetAttribLocation(texturedDrawProgram, "vertexTextureCoordinate");
	texturedWorldViewProjectionMatrixUniform = glGetUniformLocation(texturedDrawProgram, "worldViewProjectionMatrix");
	texturedNormalMatrixUniform = glGetUniformLocation(texturedDrawProgram, "normalMatrix");
	texturedLightDirectionUniform = glGetUniformLocation(texturedDrawProgram, "lightDirection");
	texturedLightColorUniform = glGetUniformLocation(texturedDrawProgram, "lightColor");
	textureRegionUniform = glGetUniformLocation(texturedDrawProgram, "textureRegion");
	textureScaleUniform = glGetUniformLocation(texturedDrawProgram, "textureScale");

	glUseProgram(drawProgram);

	currentTime = glfwGetTime();
}

GLRenderer::~GLRenderer() {
	glDeleteProgram(drawProgram);
	if (haveTexture) glDeleteTextures(1, &texture);
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
		const Material* material = e->getGeometry()->getMaterial();
		const Mesh* mesh = e->getGeometry()->getMesh();
		auto result = meshCache.find(mesh);
		if (result == meshCache.end()) {
			shared_ptr<GLPerMesh> perMesh;
			if (material->isTextured()) {
				perMesh = make_shared<GLPerMesh>(mesh,
					texturedVertexPosition, texturedVertexNormal,
					texturedVertexTextureCoordinate);
			} else {
				perMesh = make_shared<GLPerMesh>(mesh,
					vertexPosition, vertexNormal, -1);
			}
			meshCache[mesh] = perMesh;
		}

		glm::mat4 worldMatrix = e->getNode()->getWorldMatrix()
			* e->getScaleMatrix();
		glm::mat4 worldViewProjectionMatrix = camera->getProjectionMatrix()
			* camera->getViewMatrix() * worldMatrix;
		glm::mat3 normalMatrix =
			glm::mat3(glm::transpose(glm::inverse(worldMatrix)));

		if (material->isTextured()) {
			glUseProgram(texturedDrawProgram);
			glUniformMatrix4fv(texturedWorldViewProjectionMatrixUniform, 1, GL_FALSE,
				glm::value_ptr(worldViewProjectionMatrix));
			glUniformMatrix3fv(texturedNormalMatrixUniform, 1, GL_FALSE,
				glm::value_ptr(normalMatrix));
			glUniform3fv(texturedLightDirectionUniform, 1,
				glm::value_ptr(lightSources[0]->getDirection()));
			glUniform3fv(texturedLightColorUniform, 1,
				glm::value_ptr(lightSources[0]->getColor()));
			TextureRect region = textureAtlas->getRegion(material->getTextureName());
			glUniform4fv(textureRegionUniform, 1, (const GLfloat*)&region);
			glUniform2fv(textureScaleUniform, 1, glm::value_ptr(material->getTextureScale()));
		} else {
			glUseProgram(drawProgram);
			glUniformMatrix4fv(worldViewProjectionMatrixUniform, 1, GL_FALSE,
				glm::value_ptr(worldViewProjectionMatrix));
			glUniformMatrix3fv(normalMatrixUniform, 1, GL_FALSE,
				glm::value_ptr(normalMatrix));
			glUniform3fv(entityColorUniform, 1,
				glm::value_ptr(e->getGeometry()->getMaterial()->getColor()));
			glUniform3fv(lightDirectionUniform, 1,
				glm::value_ptr(lightSources[0]->getDirection()));
			glUniform3fv(lightColorUniform, 1,
				glm::value_ptr(lightSources[0]->getColor()));
		}

		shared_ptr<GLPerMesh> perMesh = meshCache[mesh];
		perMesh->bind();
		perMesh->draw();
	}
	if (particleSystem) particleSystem->draw(*camera);
	window.present();

	double seconds = glfwGetTime();
	float delta = (float)(seconds - currentTime);
	currentTime = seconds;
}

void GLRenderer::setTextureAtlas(const Engine::TextureAtlas* atlas) {
	Renderer::setTextureAtlas(atlas);
	if (haveTexture) glDeleteTextures(1, &texture);
	texture = createTexture(atlas->getTexture());
	haveTexture = true;
}
