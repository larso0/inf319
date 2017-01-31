#ifndef SCENE_CAMERA_H
#define SCENE_CAMERA_H

#include "Node.h"

namespace Scene {
	class Camera : public Node {
	public:
		Camera(Node* parent = nullptr) : Node(parent) {}
		virtual ~Camera() {}

		virtual void update() override;

		const glm::mat4& getViewMatrix() {
			return viewMatrix;
		}

	protected:
		glm::mat4 viewMatrix;

	};
}

#endif
