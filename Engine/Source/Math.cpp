#include <Engine/Math.h>

using namespace glm;

namespace Engine {
	vec3 quatTransform(const quat& q, const vec3& v) {
		quat tmp = q * quat(0.f, v) * conjugate(q);
		return vec3(tmp.x, tmp.y, tmp.z);
	}
}
