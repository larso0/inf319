#ifndef RENDER_RENDEROBJECT_H
#define RENDER_RENDEROBJECT_H

#include <Render/Mesh.h>
#include <Scene/Node.h>

namespace Render {
	class RenderObject {
	public:
		RenderObject() : mesh(nullptr), node(nullptr) {}
		RenderObject(Mesh* m, Scene::Node* n) : mesh(m), node(n) {}
		~RenderObject() {}

		Mesh* getMesh() {
			return mesh;
		}

		const Mesh* getMesh() const {
			return mesh;
		}

		Scene::Node* getNode() {
			return node;
		}

		const Scene::Node* getNode() const {
			return node;
		}

		const glm::mat4& getScaleMatrix() const {
			return scaleMatrix;
		}

		void setMesh(Mesh* m) {
			mesh = m;
		}

		void setNode(Scene::Node* n) {
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
		Scene::Node* node;
		glm::mat4 scaleMatrix;
	};
}

#endif
