#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

#include <Engine/Math.h>

namespace Engine {
	class Material {
	public:
		Material() {}

		void setColor(float r, float g, float b, float a) {
			color.r = r;
			color.g = g;
			color.b = b;
			color.a = a;
		}

		void setColor(const glm::vec4& c) {
			color = c;
		}

		const glm::vec4& getColor() const {
			return color;
		}

	private:
		glm::vec4 color;
	};
}

#endif
