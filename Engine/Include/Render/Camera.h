#ifndef RENDER_CAMERA_H
#define RENDER_CAMERA_H

#include <Scene/Node.h>

namespace Render {
	class Camera {
	public:
		Camera(Scene::Node* node = nullptr) : node(node) {}

		Scene::Node* getNode() {
			return node;
		}

		const Scene::Node* getNode() const {
			return node;
		}

		const glm::mat4& getViewMatrix() const {
			return viewMatrix;
		}

		const glm::mat4& getProjectionMatrix() const {
			return projectionMatrix;
		}

		void setNode(Scene::Node* n) {
			node = n;
		}

		void setPerspectiveProjection(float fov, float ratio, float near,
			float far) {
			projectionMatrix = glm::perspective(fov, ratio, near, far);
		}

		void setOrthoProjection(float left, float right, float top,
			float bottom, float near, float far) {
			projectionMatrix = glm::ortho(left, right, top, bottom, near, far);
		}

		void update() {
			if (node) {
				glm::vec3 dir = Math::quatTransform(node->getOrientation(),
					glm::vec3(0.f, 0.f, -1.f));
				glm::vec3 up = Math::quatTransform(node->getOrientation(),
					glm::vec3(0.f, 1.f, 0.f));
				viewMatrix = glm::lookAt(node->getPosition(),
					node->getPosition() + dir, up);
			} else {
				viewMatrix = glm::mat4();
			}
		}

	private:
		Scene::Node* node;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};
}

#endif
