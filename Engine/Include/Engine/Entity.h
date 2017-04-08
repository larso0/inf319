#ifndef ENGINE_ENTITY_H
#define ENGINE_ENTITY_H

#include "Mesh.h"
#include "Node.h"
#include "Material.h"

namespace Engine {
	class Entity {
	public:
		Entity() : mesh(nullptr), node(nullptr), material(nullptr) {}
		Entity(Mesh* mesh, Node* node, Material* material) :
			mesh(mesh), node(node), material(material) {}
		~Entity() {}

		Mesh* getMesh() {
			return mesh;
		}

		const Mesh* getMesh() const {
			return mesh;
		}

		Node* getNode() {
			return node;
		}

		const Node* getNode() const {
			return node;
		}

		Material* getMaterial() {
			return material;
		}

		const Material* getMaterial() const {
			return material;
		}

		const glm::mat4& getScaleMatrix() const {
			return scaleMatrix;
		}

		void setMesh(Mesh* m) {
			mesh = m;
		}

		void setNode(Node* n) {
			node = n;
		}

		void setMaterial(Material* m) {
			material = m;
		}

		void setScale(float x, float y, float z) {
			scaleMatrix = glm::scale(glm::mat4(), glm::vec3(x, y, z));
		}

		void setScale(const glm::vec3& s) {
			scaleMatrix = glm::scale(glm::mat4(), s);
		}

	private:
		Mesh* mesh;
		Node* node;
		Material* material;
		glm::mat4 scaleMatrix;
	};
}

#endif
