#ifndef ENGINE_RENDERER_H
#define ENGINE_RENDERER_H

#include "Camera.h"
#include "Entity.h"
#include "LightSource.h"
#include "TextureAtlas.h"
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

		virtual void setTextureAtlas(const TextureAtlas* atlas) {
			textureAtlas = atlas;
		}

		Camera* getCamera() {
			return camera;
		}

		const Camera* getCamera() const {
			return camera;
		}

		const TextureAtlas* getTextureAtlas() const {
			return textureAtlas;
		}

	protected:
		Camera* camera;
		const TextureAtlas* textureAtlas;
		std::vector<Entity*> entities;
		std::vector<LightSource*> lightSources;
	};
}

#endif
