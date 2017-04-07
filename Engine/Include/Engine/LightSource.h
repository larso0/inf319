#ifndef ENGINE_LIGHTSOURCE_H
#define ENGINE_LIGHTSOURCE_H

#include <Engine/Math.h>

namespace Engine {
	class LightSource {
	public:
		LightSource() : color(glm::vec3(1.f, 1.f, 1.f)) {}
		virtual ~LightSource() {}

		void setDirection(float x, float y, float z) {
			direction = glm::normalize(glm::vec3(x, y, z));
		}

		void setDirection(const glm::vec3& d) {
			direction = glm::normalize(d);
		}

		const glm::vec3& getDirection() const {
			return direction;
		}

		void setColor(float x, float y, float z) {
			color = glm::vec3(x, y, z);
		}

		void setColor(const glm::vec3& d) {
			color = d;
		}

		const glm::vec3& getColor() const {
			return color;
		}

	private:
		glm::vec3 direction;
		glm::vec3 color;
	};
}

#endif
