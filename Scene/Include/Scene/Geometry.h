#ifndef SCENE_GEOMETRY_H
#define SCENE_GEOMETRY_H

#include "Vertex.h"
#include <vector>

namespace Scene {
	class Geometry {
	public:
		virtual ~Geometry() {}

		uint32_t addVertex(const Vertex& vertex) {
			uint32_t i = vertices.size();
			vertices.push_back(vertex);
			return i;
		}

		std::vector<Vertex>& getVertices() {
			return vertices;
		}

	private:
		std::vector<Vertex> vertices;
	};
}

#endif
