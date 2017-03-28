#ifndef ENGINE_RENDERER_H
#define ENGINE_RENDERER_H

#include <Engine/Camera.h>
#include <Engine/Entity.h>
#include <Engine/LightSource.h>
#include <vector>

namespace Engine {
	class Renderer {
	public:
		Renderer() : camera(nullptr) {}
		virtual ~Renderer() {}

		virtual void render() = 0;

		void addEntity(Entity* e) {
			entities.push_back(e);
		}

		void addLightSource(LightSource* l) {
			lightSources.push_back(l);
		}

		void setCamera(Camera* c) {
			camera = c;
		}

		Camera* getCamera() {
			return camera;
		}

		const Camera* getCamera() const {
			return camera;
		}

	protected:
		Camera* camera;
		std::vector<Entity*> entities;
		std::vector<LightSource*> lightSources;
	};
}

#endif
