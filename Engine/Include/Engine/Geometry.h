#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "Mesh.h"
#include "Material.h"

namespace Engine {
	class Geometry {
	public:
		Geometry() : mesh(nullptr), material(nullptr) {}
		Geometry(Mesh* mesh, Material* material) :
			mesh(mesh), material(material) {}
		~Geometry() {}
		
				Mesh* getMesh() {
			return mesh;
		}

		const Mesh* getMesh() const {
			return mesh;
		}

		Material* getMaterial() {
			return material;
		}

		const Material* getMaterial() const {
			return material;
		}

		void setMesh(Mesh* m) {
			mesh = m;
		}

		void setMaterial(Material* m) {
			material = m;
		}
		
	private:
		Mesh* mesh;
		Material* material;
	};
}

#endif
