#include "GLWindow.h"
#include "GLRenderer.h"
#include <stdexcept>

using namespace std;

GLWindow::GLWindow() :
handle(nullptr),
width(800),
height(600),
mouse({false, glm::vec2(), glm::vec2()}),
open(false),
renderer(nullptr),
haveResized(false)
{
}

GLWindow::~GLWindow() {
	close();
}

void GLWindow::init() {
	handle = glfwCreateWindow(width, height, "Sandbox", nullptr, nullptr);
	if (!handle) {
		throw runtime_error("Failed to create window.");
	}

	glfwMakeContextCurrent(handle);

	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		glfwDestroyWindow(handle);
		throw runtime_error("Failed to load OpenGL extensions.");
	}

	glfwSetWindowUserPointer(handle, this);
	glfwSetKeyCallback(handle, keyCallback);
	glfwSetCharCallback(handle, charCallback);
	glfwSetMouseButtonCallback(handle, mouseButtonCallback);
	glfwSetCursorPosCallback(handle, cursorPositionCallback);
	glfwSetCursorEnterCallback(handle, cursorEnterCallback);
	glfwSetWindowSizeCallback(handle, windowSizeCallback);
	glfwSetDropCallback(handle, fileDropCallback);

	double x, y;
	glfwGetCursorPos(handle, &x, &y);
	mouse.position = glm::vec2(x, y);

	glfwSwapInterval(1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);
	glViewport(0, 0, width, height);
	open = true;
}

void GLWindow::close() {
	if (!open) return;
	glfwDestroyWindow(handle);
}

bool GLWindow::shouldClose() const {
	return glfwWindowShouldClose(handle) != 0;
}

void GLWindow::resize(uint32_t w, uint32_t h) {
	width = w;
	height = h;
	if (handle) {
		glfwSetWindowSize(handle, w, h);
	}
}

glm::vec2 GLWindow::mouseMotion() {
	glm::vec2 motion = mouse.motion;
	mouse.motion = glm::vec2();
	return motion;
}

bool GLWindow::isCursorHidden() const {
	return mouse.hidden;
}

void GLWindow::toggleCursorHidden() {
	glfwSetInputMode(handle, GLFW_CURSOR,
		mouse.hidden ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	mouse.hidden = !mouse.hidden;
}

void GLWindow::handleEvents() {
	glfwPollEvents();
	if (haveResized) {
		glViewport(0, 0, width, height);
		haveResized = false;
	}
}

Engine::KeyAction GLWindow::getKey(Engine::Key key) const {
	return static_cast<Engine::KeyAction>(glfwGetKey(handle,
		static_cast<int>(key)));
}

Engine::KeyAction GLWindow::getMouseButton(Engine::MouseButton btn) const {
	return static_cast<Engine::KeyAction>(glfwGetMouseButton(handle,
		static_cast<int>(btn)));
}

glm::vec2 GLWindow::getCursorPosition() const {
	return mouse.position;
}

uint32_t GLWindow::getWidth() const {
	return width;
}

uint32_t GLWindow::getHeight() const {
	return height;
}

Engine::Renderer& GLWindow::getRenderer() {
	if (!renderer) {
		renderer = new GLRenderer(*this);
	}
	return *renderer;
}

void GLWindow::keyCallback(GLFWwindow* handle, int key, int, int action, int mods) {
	GLWindow& window = *((GLWindow*)glfwGetWindowUserPointer(handle));
	switch (action) {
	case GLFW_RELEASE:
		for (Engine::KeyEventHandler* handler : window.keyEventHandlers) {
			handler->keyRelease(static_cast<Engine::Key>(key),
				static_cast<Engine::Modifier>(mods));
		}
		break;
	case GLFW_PRESS:
		for (Engine::KeyEventHandler* handler : window.keyEventHandlers) {
			handler->keyPress(static_cast<Engine::Key>(key),
				static_cast<Engine::Modifier>(mods));
		}
		break;
	case GLFW_REPEAT:
		for (Engine::KeyEventHandler* handler : window.keyEventHandlers) {
			handler->keyRelease(static_cast<Engine::Key>(key),
				static_cast<Engine::Modifier>(mods));
		}
		break;
	default:
		break;
	}
}

void GLWindow::charCallback(GLFWwindow* handle, unsigned int codepoint) {
	GLWindow& window = *((GLWindow*)glfwGetWindowUserPointer(handle));
	for (Engine::KeyEventHandler* handler : window.keyEventHandlers) {
		handler->charInput((uint32_t)codepoint);
	}
}

void GLWindow::mouseButtonCallback(GLFWwindow* handle, int button, int action, int mods) {
	GLWindow& window = *((GLWindow*)glfwGetWindowUserPointer(handle));
	switch (action) {
	case GLFW_RELEASE:
		for (Engine::MouseEventHandler* handler : window.mouseEventHandlers) {
			handler->buttonRelease(static_cast<Engine::MouseButton>(button),
				static_cast<Engine::Modifier>(mods));
		}
		break;
	case GLFW_PRESS:
		for (Engine::MouseEventHandler* handler : window.mouseEventHandlers) {
			handler->buttonPress(static_cast<Engine::MouseButton>(button),
				static_cast<Engine::Modifier>(mods));
		}
		break;
	default:
		break;
	}
}

void GLWindow::cursorPositionCallback(GLFWwindow* handle, double x, double y) {
	GLWindow& window = *((GLWindow*)glfwGetWindowUserPointer(handle));

	for (Engine::MouseEventHandler* handler : window.mouseEventHandlers) {
		handler->cursorPosition(x, y);
	}

	glm::vec2 position(x, y);
	glm::vec2 motion = position - window.mouse.position;
	if (window.mouse.hidden) {
		window.mouse.motion = position - window.mouse.position;
	}
	window.mouse.position = position;
}

void GLWindow::cursorEnterCallback(GLFWwindow* handle, int entered) {
	GLWindow& window = *((GLWindow*)glfwGetWindowUserPointer(handle));
	if (entered) {
		for (Engine::MouseEventHandler* handler : window.mouseEventHandlers) {
			handler->cursorEnter();
		}
	} else {
		for (Engine::MouseEventHandler* handler : window.mouseEventHandlers) {
			handler->cursorLeave();
		}
	}
}

void GLWindow::windowSizeCallback(GLFWwindow* handle, int w, int h) {
	GLWindow& window = *((GLWindow*)glfwGetWindowUserPointer(handle));
	for (Engine::WindowEventHandler* handler : window.windowEventHandlers) {
		handler->resize(w, h);
	}
	window.width = (uint32_t)w;
	window.height = (uint32_t)h;
	window.haveResized = true;
}

void GLWindow::fileDropCallback(GLFWwindow* handle, int count, const char** cPaths) {
	GLWindow& window = *((GLWindow*)glfwGetWindowUserPointer(handle));
	vector<string> paths;
	for (int i = 0; i < count; i++) {
		paths.push_back(string(cPaths[i]));
	}
	for (Engine::WindowEventHandler* handler : window.windowEventHandlers) {
		handler->fileDrop(paths);
	}
}
