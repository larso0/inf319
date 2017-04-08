#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

#include "Renderer.h"
#include "Input.h"
#include "Math.h"
#include "EventHandler.h"
#include "KeyEventHandler.h"
#include "MouseEventHandler.h"
#include "WindowEventHandler.h"
#include <vector>
#include <cstdint>

namespace Engine {
	class Window {
	public:
		virtual ~Window() {}

		virtual void init() = 0;
		virtual void close() = 0;
		virtual bool shouldClose() const = 0;
		virtual void resize(uint32_t w, uint32_t h) = 0;
		virtual glm::vec2 mouseMotion() = 0;
		virtual bool isCursorHidden() const = 0;
		virtual void toggleCursorHidden() = 0;
		virtual KeyAction getKey(Key key) const = 0;
		virtual KeyAction getMouseButton(MouseButton btn) const = 0;
		virtual glm::vec2 getCursorPosition() const = 0;
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
		virtual Renderer& getRenderer() = 0;

		void addEventHandler(EventHandler* handler) {
			if (dynamic_cast<KeyEventHandler*>(handler)) {
				keyEventHandlers.push_back(static_cast<KeyEventHandler*>(handler));
			} else if (dynamic_cast<MouseEventHandler*>(handler)) {
				mouseEventHandlers.push_back(static_cast<MouseEventHandler*>(handler));
			} else if (dynamic_cast<WindowEventHandler*>(handler)) {
				windowEventHandlers.push_back(static_cast<WindowEventHandler*>(handler));
			}
		}

	protected:
		std::vector<KeyEventHandler*> keyEventHandlers;
		std::vector<MouseEventHandler*> mouseEventHandlers;
		std::vector<WindowEventHandler*> windowEventHandlers;
	};
}

#endif
