#ifndef SCENE_GEOMETRY_H
#define SCENE_GEOMETRY_H

#include "Vertex.h"
#include <vector>

namespace Scene {
	class Geometry {
	public:
		enum class PrimitiveType {
			Points,
			Lines,
			LineLoop,
			LineStrip,
			Triangles,
			TriangleStrip,
			TriangleFan
		};

		Geometry(PrimitiveType pt = PrimitiveType::Triangles) :
			primitiveType(pt) {}
		virtual ~Geometry() {}

		uint32_t addVertex(const Vertex& vertex) {
			uint32_t i = vertices.size();
			vertices.push_back(vertex);
			return i;
		}

		void setPrimitiveType(PrimitiveType pt) {
			primitiveType = pt;
		}

		PrimitiveType getPrimitiveType() const {
			return primitiveType;
		}

		std::vector<Vertex>& getVertices() {
			return vertices;
		}

		size_t getVertexDataSize() const {
			return sizeof(Vertex)*vertices.size();
		}

		const Vertex* getVertexData() const {
			return vertices.data();
		}

		Vertex* getVertexData() {
			return vertices.data();
		}

		virtual size_t getElementCount() const {
			return vertices.size();
		}

	private:
		PrimitiveType primitiveType;
		std::vector<Vertex> vertices;
	};
}

#endif
