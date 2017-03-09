#include <Engine/Camera.h>
#include <Engine/Entity.h>
#include <Engine/MeshGeneration.h>
#include <Engine/Renderer.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <Engine/Node.h>
#include <vector>
#include <unordered_map>
#include <memory>

using namespace std;
using namespace Engine;

class Configuration {
public:
	Configuration() :
		width(800), height(600),
		mouseHidden(false),
		mouseX(0.f), mouseY(0.f),
		sensitivity(0.002f),
		yaw(0.f), pitch(0.f),
		movementSpeed(2.f) {}

	int width, height;
	bool mouseHidden;
	double mouseX, mouseY;
	float sensitivity;
	float yaw, pitch;
	float movementSpeed;
};

void errorCallback(int error, const char* description) {
	cerr << "Error: " << description << endl;
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
	Configuration* config = (Configuration*)glfwGetWindowUserPointer(window);
	if (action != GLFW_RELEASE) return;
	switch (key) {
	case GLFW_KEY_ESCAPE:
		glfwSetInputMode(window, GLFW_CURSOR,
			config->mouseHidden ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		config->mouseHidden = ! config->mouseHidden;
		break;
	default:
		break;
	}
}

void mousePositionCallback(GLFWwindow* window, double x, double y) {
	Configuration* config = (Configuration*)glfwGetWindowUserPointer(window);
	float motionX = x - config->mouseX;
	float motionY = y - config->mouseY;
	if (config->mouseHidden) {
		config->yaw -= motionX*config->sensitivity;
		config->pitch -= motionY*config->sensitivity;
	}
	config->mouseX = x;
	config->mouseY = y;
}

const char* vertexShaderSource =
	"#version 450\n"
	"in vec3 vertexPosition;\n"
	"in vec3 vertexNormal;\n"
	"out vec3 fragmentNormal;\n"
	"uniform mat4 worldViewProjectionMatrix;\n"
	"uniform mat4 normalMatrix;\n"
	"void main() {\n"
	"	fragmentNormal = normalize(normalMatrix * vec4(vertexNormal, 0)).xyz;\n"
	"	gl_Position = worldViewProjectionMatrix * vec4(vertexPosition, 1);\n"
	"}\n";

const char* fragmentShaderSource =
	"#version 450\n"
	"in vec3 fragmentNormal;\n"
	"out vec3 color;\n"
	"void main() {\n"
	"	color = fragmentNormal;\n"
	"}\n";

GLuint createShader(GLenum type, const char* source) {
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

GLuint createDrawProgram() {
	GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
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

GLenum primitiveType(Mesh::Topology pt) {
	switch (pt) {
	case Mesh::Topology::Points: return GL_POINTS;
	case Mesh::Topology::Lines: return GL_LINES;
	case Mesh::Topology::LineStrip: return GL_LINE_STRIP;
	case Mesh::Topology::Triangles: return GL_TRIANGLES;
	case Mesh::Topology::TriangleStrip: return GL_TRIANGLE_STRIP;
	case Mesh::Topology:: TriangleFan: return GL_TRIANGLE_FAN;
	}
	throw invalid_argument("Unknown primitive type");
}

GLuint createVertexBuffer(const Mesh& mesh) {
	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(
		GL_ARRAY_BUFFER,
		mesh.getVertexDataSize(),
		mesh.getVertexData(),
		GL_STATIC_DRAW);
	return buf;
}

GLuint createIndexBuffer(const IndexedMesh& mesh) {
	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(
		GL_ARRAY_BUFFER,
		mesh.getIndexDataSize(),
		mesh.getIndexData(),
		GL_STATIC_DRAW);
	return buf;
}

GLuint createVAO(
	GLuint vertexBuffer,
	GLint vertexPosition,
	GLint vertexNormal,
	GLint vertexTextureCoordinate)
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	if (vertexPosition >= 0) {
		glEnableVertexAttribArray(vertexPosition);
		glVertexAttribPointer(vertexPosition, 3, GL_FLOAT, GL_FALSE,
			Vertex::Stride, (const GLvoid*) Vertex::PositionOffset);
	}

	if (vertexNormal >= 0) {
		glEnableVertexAttribArray(vertexNormal);
		glVertexAttribPointer(vertexNormal, 3, GL_FLOAT, GL_FALSE,
			Vertex::Stride, (const GLvoid*) Vertex::NormalOffset);
	}

	if (vertexTextureCoordinate >= 0) {
		glEnableVertexAttribArray(vertexTextureCoordinate);
		glVertexAttribPointer(vertexTextureCoordinate, 2, GL_FLOAT, GL_FALSE,
			Vertex::Stride, (const GLvoid*) Vertex::TextureCoordinateOffset);
	}

	return vao;
}

GLuint createVAOIndexed(
	GLuint vertexBuffer,
	GLuint indexBuffer,
	GLint vertexPosition,
	GLint vertexNormal,
	GLint vertexTextureCoordinate)
{
	GLuint vao = createVAO(vertexBuffer, vertexPosition, vertexNormal,
		vertexTextureCoordinate);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	return vao;
}

class GLRenderer : public Renderer {
public:
	GLRenderer() {
		drawProgram = createDrawProgram();
		vertexPosition = glGetAttribLocation(drawProgram, "vertexPosition");
		vertexNormal = glGetAttribLocation(drawProgram, "vertexNormal");
		vertexTextureCoordinate = glGetAttribLocation(drawProgram, "vertexTextureCoordinate");
		worldViewProjectionMatrixUniform = glGetUniformLocation(drawProgram, "worldViewProjectionMatrix");
		normalMatrixUniform = glGetUniformLocation(drawProgram, "normalMatrix");
		glUseProgram(drawProgram);
	}

	~GLRenderer() {
		glDeleteProgram(drawProgram);
	}

	void render(const Camera& camera, const vector<Entity>& entities) override {
		for (const Entity& e : entities) {
			const Mesh* mesh = e.getMesh();
			auto result = meshCache.find(mesh);
			if (result == meshCache.end()) {
				shared_ptr<MeshGLObjects> glObjects = make_shared<MeshGLObjects>();
				glObjects->primitiveType = primitiveType(mesh->getTopology());
				GLuint vertexBuffer = createVertexBuffer(*mesh);
				glObjects->buffers.push_back(vertexBuffer);
				const IndexedMesh* indexedMesh = dynamic_cast<const IndexedMesh*>(mesh);
				if (indexedMesh) {
					GLuint indexBuffer = createIndexBuffer(*indexedMesh);
					glObjects->buffers.push_back(indexBuffer);
					glObjects->vao = createVAOIndexed(vertexBuffer, indexBuffer,
						vertexPosition, vertexNormal, vertexTextureCoordinate);
					glObjects->drawFunction = drawElements;
				} else {
					glObjects->vao = createVAO(vertexBuffer, vertexPosition,
						vertexNormal, vertexTextureCoordinate);
					glObjects->drawFunction = drawArrays;
				}
				meshCache[mesh] = glObjects;
			}

			glm::mat4 worldMatrix = e.getNode()->getWorldMatrix() * e.getScaleMatrix();
			glm::mat4 worldViewProjectionMatrix = camera.getProjectionMatrix() * camera.getViewMatrix() * worldMatrix;
			glm::mat4 normalMatrix =
				glm::transpose(glm::inverse(worldMatrix));

			glUniformMatrix4fv(worldViewProjectionMatrixUniform, 1, GL_FALSE,
				glm::value_ptr(worldViewProjectionMatrix));
			glUniformMatrix4fv(normalMatrixUniform, 1, GL_FALSE,
				glm::value_ptr(normalMatrix));

			shared_ptr<MeshGLObjects> glObjects = meshCache[mesh];
			glBindVertexArray(glObjects->vao);
			glObjects->drawFunction(glObjects->primitiveType, mesh->getElementCount());
		}

	}

private:
	class MeshGLObjects {
	public:
		MeshGLObjects() : primitiveType(GL_TRIANGLES), vao(0), drawFunction(nullptr) {}
		~MeshGLObjects() {
			glDeleteBuffers(buffers.size(), buffers.data());
			glDeleteVertexArrays(1, &vao);
		}

		GLenum primitiveType;
		vector<GLuint> buffers;
		GLuint vao;
		void (*drawFunction)(GLenum, GLsizei);
	};

	static void drawArrays(GLenum primitiveType, GLsizei count) {
		glDrawArrays(primitiveType, 0, count);
	}

	static void drawElements(GLenum primitiveType, GLsizei count) {
		glDrawElements(primitiveType, count, GL_UNSIGNED_INT, 0);
	}

	GLuint drawProgram;
	GLint vertexPosition, vertexNormal, vertexTextureCoordinate;
	GLint worldViewProjectionMatrixUniform, normalMatrixUniform;
	unordered_map<const Mesh*, shared_ptr<MeshGLObjects>> meshCache;
};

int main(int argc, char** argv) {
	if (!glfwInit()) {
		cerr << "Failed to initialize GLFW.\n";
		return 1;
	}
	glfwSetErrorCallback(errorCallback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	Configuration config;

	GLFWwindow* window = glfwCreateWindow(config.width, config.height, "Scratchpad", NULL, NULL);
	if (!window) {
		cerr << "Unable to create window.\n";
		glfwTerminate();
		return 2;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		cerr << "Could not load OpenGL extensions.\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return 3;
	}

	glfwSetWindowUserPointer(window, &config);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mousePositionCallback);
	glfwGetCursorPos(window, &config.mouseX, &config.mouseY);

	glfwSwapInterval(1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);
	glViewport(0, 0, config.width, config.height);

	GLRenderer renderer;

	Mesh cubeMesh = generateCube();
	IndexedMesh sphereMesh = generateSphere(5);

	Node cube1;
	Node cube2(&cube1);
	Node sphere(&cube1);
	cube2.translate(2.f, 0.f, 0.f);
	cube2.rotate(glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));
	sphere.translate(0.f, 2.f, 0.f);
	cube1.update();

	vector<Entity> entities{
		Entity(&cubeMesh, &cube1),
		Entity(&cubeMesh, &cube2),
		Entity(&sphereMesh, &sphere)
	};
	entities[1].setScale(0.2f, 2.f, 0.2f);

	Node cameraNode;
	cameraNode.translate(0.f, 0.f, 3.f);
	cameraNode.update();

	Camera camera(&cameraNode);
	camera.setPerspectiveProjection(glm::radians(60.f), 4.f/3.f, 0.1f, 100.f);
	camera.update();

	double time = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderer.render(camera, entities);

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (config.mouseHidden) {
			glm::vec3 cameraDirection = quatTransform(
				cameraNode.getOrientation(), glm::vec3(0.f, 0.f, -1.f));
			glm::vec3 cameraRight = quatTransform(cameraNode.getOrientation(),
				glm::vec3(1.f, 0.f, 0.f));

			double seconds = glfwGetTime();
			float delta = seconds - time;
			time = seconds;

			glm::vec3 movement;
			bool moved = false;
			bool keyW = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
			bool keyA = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
			bool keyS = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
			bool keyD = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
			bool keyQ = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
			bool keyE = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
			if (keyW && !keyS) {
				movement += cameraDirection;
				moved = true;
			} else if (keyS && !keyW) {
				movement -= cameraDirection;
				moved = true;
			}
			if (keyA && !keyD) {
				movement -= cameraRight;
				moved = true;
			} else if (keyD && !keyA) {
				movement += cameraRight;
				moved = true;
			}
			if (keyQ && !keyE) {
				movement -= glm::vec3(0.f, 1.f, 0.f);
				moved = true;
			} else if (keyE && !keyQ) {
				movement += glm::vec3(0.f, 1.f, 0.f);
				moved = true;
			}

			if (moved) {
				movement = glm::normalize(movement) * delta * config.movementSpeed;
				cameraNode.translate(movement);
			}

			cameraNode.setRotation(config.yaw, glm::vec3(0.f, 1.f, 0.f));
			cameraNode.rotate(config.pitch, glm::vec3(1.f, 0.f, 0.f));
			cameraNode.update();
			camera.update();

		}
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
