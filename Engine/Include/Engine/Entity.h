#ifndef ENGINE_ENTITY_H
#define ENGINE_ENTITY_H

#include <Engine/Mesh.h>
#include <Engine/Node.h>

namespace Engine {
	class Entity {
	public:
		Entity() : mesh(nullptr), node(nullptr) {}
		Entity(Mesh* m, Engine::Node* n) : mesh(m), node(n) {}
		~Entity() {}

		Mesh* getMesh() {
			return mesh;
		}

		const Mesh* getMesh() const {
			return mesh;
		}

		Engine::Node* getNode() {
			return node;
		}

		const Engine::Node* getNode() const {
			return node;
		}

		const glm::mat4& getScaleMatrix() const {
			return scaleMatrix;
		}

		void setMesh(Mesh* m) {
			mesh = m;
		}

		void setNode(Engine::Node* n) {
			node = n;
		}

		void setScale(float x, float y, float z) {
			scaleMatrix = glm::scale(glm::mat4(), glm::vec3(x, y, z));
		}

		void setScale(const glm::vec3& s) {
			scaleMatrix = glm::scale(glm::mat4(), s);
		}

	private:
		Mesh* mesh;
		Engine::Node* node;
		glm::mat4 scaleMatrix;
	};
}

#endif
