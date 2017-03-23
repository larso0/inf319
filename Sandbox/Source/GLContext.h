#ifndef GLCONTEXT_H
#define GLCONTEXT_H

#include <Engine/Context.h>

class GLContext : public Engine::Context {
public:
	GLContext();
	~GLContext();

	Engine::Window& createWindow(int w, int h, int flags) override;

private:
	Engine::Window* window;
};

#endif
