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

	private:
		PrimitiveType primitiveType;
		std::vector<Vertex> vertices;
	};
}

#endif
