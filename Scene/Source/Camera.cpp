#include <Scene/Camera.h>

using glm::vec3;
using Math::quatTransform;
using glm::lookAt;

namespace Scene {
	void Camera::update() {
		Node::update();
		vec3 direction = quatTransform(orientation, vec3(0.f, 0.f, -1.f));
		vec3 up = quatTransform(orientation, vec3(0.f, 1.f, 0.f));
		viewMatrix = lookAt(position, position + direction, up);
	}
}
