#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

#include "Math.h"
#include "Texture.h"

namespace Engine {
	class Material {
	public:
		Material() : texture(nullptr) {}

		void setColor(float r, float g, float b, float a) {
			color.r = r;
			color.g = g;
			color.b = b;
			color.a = a;
		}

		void setColor(const glm::vec4& c) {
			color = c;
		}
		
		void setTexture(Texture* t) {
			texture = t;
		}

		const glm::vec4& getColor() const {
			return color;
		}
		
		Texture* getTexture() {
			return texture;
		}
		
		const Texture* getTexture() const {
			return texture;
		}

	private:
		glm::vec4 color;
		Texture* texture;
	};
}

#endif
