#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <Scene/Node.h>
#include <Scene/Camera.h>
#include <Scene/GeometryGeneration.h>

using namespace std;

void errorCallback(int error, const char* description) {
	cerr << "Error: " << description << endl;
}

const char* vertexShaderSource =
	"#version 450\n"
	"in vec3 vertexPosition;\n"
	"in vec3 vertexNormal;\n"
	"out vec3 fragmentNormal;\n"
	"uniform mat4 worldViewProjectionMatrix;\n"
	"void main() {\n"
	"	fragmentNormal = vertexNormal;\n"
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

int main(int argc, char** argv) {
	if (!glfwInit()) {
		cerr << "Failed to initialize GLFW.\n";
		return 1;
	}
	glfwSetErrorCallback(errorCallback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Scratchpad", NULL, NULL);
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

	glfwSwapInterval(1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);
	glViewport(0, 0, 800, 600);

	GLuint drawProgram = createDrawProgram();
	Scene::Geometry cubeGeometry = Scene::generateCube();
	GLuint cubeVertexBuffer;
	glGenBuffers(1, &cubeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(Scene::Vertex)*cubeGeometry.getVertices().size(),
		cubeGeometry.getVertices().data(),
		GL_STATIC_DRAW);

	glUseProgram(drawProgram);

	GLint vertexPosition = glGetAttribLocation(drawProgram, "vertexPosition");
	GLint vertexNormal = glGetAttribLocation(drawProgram, "vertexNormal");
	GLint worldViewProjectionMatrixUniform = glGetUniformLocation(drawProgram, "worldViewProjectionMatrix");

	GLuint cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glEnableVertexAttribArray(vertexPosition);
	glVertexAttribPointer(vertexPosition, 3, GL_FLOAT, GL_FALSE,
		sizeof(Scene::Vertex), 0);
	glEnableVertexAttribArray(vertexNormal);
	glVertexAttribPointer(vertexNormal, 3, GL_FLOAT, GL_FALSE,
		sizeof(Scene::Vertex), (const GLvoid*)sizeof(glm::vec3));

	Scene::Node cube1;
	Scene::Node cube2(&cube1);
	cube2.translate(2.f, 0.f, 0.f);
	cube2.rotate(glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));
	cube2.update();

	Scene::Camera camera;
	camera.translate(0.f, 0.f, 3.f);
	camera.update();

	glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.f), 800.f / 600.f, 0.1f, 100.f);
	glm::mat4 testScaleMatrix = glm::scale(glm::mat4(), glm::vec3(0.2f, 2.f, 0.2f));

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 worldViewProjectionMatrix =
			projectionMatrix * camera.getViewMatrix() * cube1.getWorldMatrix();
		glUniformMatrix4fv(worldViewProjectionMatrixUniform, 1, GL_FALSE,
			glm::value_ptr(worldViewProjectionMatrix));
		glDrawArrays(GL_TRIANGLES, 0, cubeGeometry.getVertices().size());

		worldViewProjectionMatrix =
			projectionMatrix * camera.getViewMatrix() * cube2.getWorldMatrix() * testScaleMatrix;
		glUniformMatrix4fv(worldViewProjectionMatrixUniform, 1, GL_FALSE,
			glm::value_ptr(worldViewProjectionMatrix));
		glDrawArrays(GL_TRIANGLES, 0, cubeGeometry.getVertices().size());

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVertexBuffer);
	glDeleteProgram(drawProgram);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
