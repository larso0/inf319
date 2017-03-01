#ifndef ENGINE_RENDERER_H
#define ENGINE_RENDERER_H

#include <Engine/Camera.h>
#include <Engine/Entity.h>
#include <vector>

namespace Engine {
	class Renderer {
	public:
		virtual ~Renderer() {}

		virtual void render(const Camera& camera,
			const std::vector<Entity>& entities) = 0;
	};
}

#endif
