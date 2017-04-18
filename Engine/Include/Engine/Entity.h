#ifndef ENGINE_ENTITY_H
#define ENGINE_ENTITY_H

#include "Node.h"
#include "Geometry.h"

namespace Engine {
	class Entity {
	public:
		Entity() : node(nullptr), geometry(nullptr) {}
		Entity(Node* node, Geometry* geometry) :
			node(node), geometry(geometry) {}
		~Entity() {}

		Node* getNode() {
			return node;
		}

		const Node* getNode() const {
			return node;
		}
		
		Geometry* getGeometry() {
			return geometry;
		}
		
		const Geometry* getGeometry() const {
			return geometry;
		}
		
		const glm::mat4& getScaleMatrix() const {
			return scaleMatrix;
		}

		void setNode(Node* n) {
			node = n;
		}
		
		void setGeometry(Geometry* g) {
			geometry = g;
		}
		
		void setScale(float x, float y, float z) {
			scaleMatrix = glm::scale(glm::mat4(), glm::vec3(x, y, z));
		}
		
		void setScale(const glm::vec3& s) {
			scaleMatrix = glm::scale(glm::mat4(), s);
		}
		
	private:
		Node* node;
		Geometry* geometry;
		glm::mat4 scaleMatrix;
	};
}

#endif
