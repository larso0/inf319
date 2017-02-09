#ifndef RENDER_RENDERER_H
#define RENDER_RENDERER_H

#include <Render/Entity.h>
#include <Render/Camera.h>
#include <vector>

namespace Render {
	class Renderer {
	public:
		virtual ~Renderer() {}

		virtual void render(const Camera& camera,
			const std::vector<Entity>& entities) = 0;
	};
}

#endif
