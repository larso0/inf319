#ifndef ENGINE_MATH_H
#define ENGINE_MATH_H

#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdint>

namespace Engine {
	glm::vec3 quatTransform(const glm::quat& q, const glm::vec3& v);
}

#endif
