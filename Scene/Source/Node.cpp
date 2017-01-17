#include <Scene/Node.h>
#include <algorithm>

using std::remove_if;

namespace Scene {
    void Node::update() {
        localMatrix = glm::mat4_cast(rotation);
        localMatrix = glm::translate(localMatrix, translation);
        if (parent) {
            worldMatrix = parent->getWorldMatrix() * localMatrix;
        } else {
            worldMatrix = localMatrix;
        }
        for (auto child : children) {
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