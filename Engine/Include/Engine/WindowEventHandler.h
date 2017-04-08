#ifndef ENGINE_WINDOWEVENTHANDLER_H
#define ENGINE_WINDOWEVENTHANDLER_H

#include "EventHandler.h"
#include <string>
#include <vector>

namespace Engine {
	class WindowEventHandler : public EventHandler {
	public:
		virtual ~WindowEventHandler() {}
		
		virtual void resize(int w, int h) {}
		virtual void fileDrop(const std::vector<std::string>& paths) {}
	};
}

#endif