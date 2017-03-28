#ifndef ENGINE_LIGHTSOURCE_H
#define ENGINE_LIGHTSOURCE_H

#include <Engine/Math.h>

namespace Engine {
	class LightSource {
	public:
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

	private:
		glm::vec3 direction;
	};
}

#endif
