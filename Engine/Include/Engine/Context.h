#ifndef ENGINE_CONTEXT_H
#define ENGINE_CONTEXT_H

#include "Window.h"

namespace Engine {
	class Context {
	public:
		virtual ~Context() {}

		virtual Window& createWindow(int w, int h, int flags) = 0;
	};
}

#endif
