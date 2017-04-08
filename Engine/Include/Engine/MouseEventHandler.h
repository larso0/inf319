#ifndef ENGINE_MOUSEEVENTHANDLER_H
#define ENGINE_MOUSEEVENTHANDLER_H

#include "EventHandler.h"
#include "Input.h"

namespace Engine {
	class MouseEventHandler : public EventHandler {
	public:
		virtual ~MouseEventHandler() {}
		
		virtual void buttonPress(MouseButton button, Modifier mods) {}
		virtual void buttonRelease(MouseButton button, Modifier mods) {}
		virtual void cursorPosition(double x, double y) {}
		virtual void cursorEnter() {}
		virtual void cursorLeave() {}
	};
}

#endif