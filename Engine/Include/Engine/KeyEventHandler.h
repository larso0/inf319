#ifndef ENGINE_KEYEVENTHANDLER_H
#define ENGINE_KEYEVENTHANDLER_H

#include "EventHandler.h"
#include "Input.h"
#include <cstdint>

namespace Engine {
	class KeyEventHandler : public EventHandler {
	public:		
		virtual void keyPress(Key key, Modifier mods) {}
		virtual void keyRelease(Key key, Modifier mods) {}
		virtual void keyRepeat(Key key, Modifier mods) {}
		virtual void charInput(uint32_t codepoint) {}
	};
}

#endif