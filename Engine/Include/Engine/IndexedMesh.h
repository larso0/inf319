#ifndef ENGINE_INDEXEDMESH_H
#define ENGINE_INDEXEDMESH_H

#include <Engine/Mesh.h>

namespace Engine {
	class IndexedMesh : public Mesh {
	public:
		IndexedMesh(Topology pt = Topology::Triangles) :
			Mesh(pt) {}

		void addIndex(uint32_t index) {
			indices.push_back(index);
		}

		void addFace(uint32_t a, uint32_t b, uint32_t c) {
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);
		}

		std::vector<uint32_t>& getIndices() {
			return indices;
		}

		size_t getIndexDataSize() const {
			return sizeof(uint32_t)*indices.size();
		}

		const uint32_t* getIndexData() const {
			return indices.data();
		}

		uint32_t* getIndexData() {
			return indices.data();
		}

		size_t getElementCount() const override {
			return indices.size();
		}

	private:
		std::vector<uint32_t> indices;
	};
}

#endif
