#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

#include "Math.h"
#include <string>

namespace Engine {
	class Material {
	public:
		Material() : textureScale(glm::vec2(1.f, 1.f)) {}

		void setColor(float r, float g, float b, float a) {
			color.r = r;
			color.g = g;
			color.b = b;
			color.a = a;
		}

		bool isTextured() const {
			return !textureName.empty();
		}

		void setColor(const glm::vec4& c) {
			color = c;
		}

		void setTextureName(const std::string& name) {
			textureName = name;
		}

		void setTextureScale(float x, float y) {
			textureScale = glm::vec2(x, y);
		}

		void setTextureScale(glm::vec2 scale) {
			textureScale = scale;
		}

		const glm::vec4& getColor() const {
			return color;
		}

		const std::string& getTextureName() const {
			return textureName;
		}

		glm::vec2 getTextureScale() const {
			return textureScale;
		}

	private:
		glm::vec4 color;
		std::string textureName;
		glm::vec2 textureScale;
	};
}

#endif
