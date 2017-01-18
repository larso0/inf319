#include <Scene/Node.h>
#include <algorithm>

using std::remove_if;
using Math::quatTransform;

namespace Scene {
	void Node::update() {
		glm::mat4 localMatrix = glm::mat4_cast(rotation);
		localMatrix = glm::translate(localMatrix, translation);

		if (parent) {
			position = parent->getPosition()
				+ quatTransform(parent->getOrientation(),
					glm::normalize(translation)) * glm::length(translation);
			orientation = parent->getOrientation() * rotation;
			worldMatrix = parent->getWorldMatrix() * localMatrix;
		} else {
			position = translation;
			orientation = rotation;
			worldMatrix = localMatrix;
		}

		for (Node* child : children) {
			child->update();
		}
	}

	void Node::addChild(Node* child) {
		if (child->parent) child->parent->removeChild(child);
		children.push_back(child);
		child->parent = this;
	}

	void Node::removeChild(Node* child) {
		auto pred = [child](Node* c) -> bool {
			if (c == child) {
				child->parent = nullptr;
				return true;
			}
			return false;
		};
		children.erase(remove_if(children.begin(), children.end(), pred));
	}
}
