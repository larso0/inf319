#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <Scene/Node.h>
#include <Scene/Camera.h>
#include <Scene/GeometryGeneration.h>

using namespace std;

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

	GLuint drawProgram = createDrawProgram();
	Scene::Geometry cubeGeometry = Scene::generateCube();
	Scene::ElementGeometry sphereGeometry = Scene::generateSphere(5);

	GLuint cubeVertexBuffer;
	glGenBuffers(1, &cubeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(Scene::Vertex)*cubeGeometry.getVertices().size(),
		cubeGeometry.getVertices().data(),
		GL_STATIC_DRAW);

	GLuint sphereVertexBuffer;
	glGenBuffers(1, &sphereVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(Scene::Vertex)*sphereGeometry.getVertices().size(),
		sphereGeometry.getVertices().data(),
		GL_STATIC_DRAW);

	GLuint sphereIndexBuffer;
	glGenBuffers(1, &sphereIndexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphereIndexBuffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(uint32_t)*sphereGeometry.getIndices().size(),
		sphereGeometry.getIndices().data(),
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

	GLuint sphereVAO;
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
	glEnableVertexAttribArray(vertexPosition);
	glVertexAttribPointer(vertexPosition, 3, GL_FLOAT, GL_FALSE,
		sizeof(Scene::Vertex), 0);
	glEnableVertexAttribArray(vertexNormal);
	glVertexAttribPointer(vertexNormal, 3, GL_FLOAT, GL_FALSE,
		sizeof(Scene::Vertex), (const GLvoid*)sizeof(glm::vec3));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndexBuffer);

	Scene::Node cube1;
	Scene::Node cube2(&cube1);
	Scene::Node sphere(&cube1);
	cube2.translate(2.f, 0.f, 0.f);
	cube2.rotate(glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));
	sphere.translate(0.f, 2.f, 0.f);
	cube1.update();

	Scene::Camera camera;
	camera.translate(0.f, 0.f, 3.f);
	camera.update();

	glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.f), 800.f / 600.f, 0.1f, 100.f);
	glm::mat4 testScaleMatrix = glm::scale(glm::mat4(), glm::vec3(0.2f, 2.f, 0.2f));

	double time = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(cubeVAO);

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

		glBindVertexArray(sphereVAO);

		worldViewProjectionMatrix =
			projectionMatrix * camera.getViewMatrix() * sphere.getWorldMatrix();
		glUniformMatrix4fv(worldViewProjectionMatrixUniform, 1, GL_FALSE,
			glm::value_ptr(worldViewProjectionMatrix));
		glDrawElements(GL_TRIANGLES, sphereGeometry.getIndices().size(), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (config.mouseHidden) {
			glm::vec3 cameraDirection = Math::quatTransform(
				camera.getOrientation(), glm::vec3(0.f, 0.f, -1.f));
			glm::vec3 cameraRight = Math::quatTransform(camera.getOrientation(),
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
				camera.translate(movement);
			}

			camera.setRotation(config.yaw, glm::vec3(0.f, 1.f, 0.f));
			camera.rotate(config.pitch, glm::vec3(1.f, 0.f, 0.f));
			camera.update();

		}
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVertexBuffer);
	glDeleteProgram(drawProgram);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
