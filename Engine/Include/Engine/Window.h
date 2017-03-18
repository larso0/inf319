#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

#include <Engine/Renderer.h>
#include <cstdint>

namespace Engine {
	class Window {
	public:
		virtual ~Window() {}

		virtual void init() = 0;
		virtual void close() = 0;
		virtual void resize(uint32_t w, uint32_t h) = 0;
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
		virtual Renderer& getRenderer() = 0;
	};
}

#endif
