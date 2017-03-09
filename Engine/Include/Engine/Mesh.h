#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include <Engine/Vertex.h>
#include <vector>

namespace Engine {
	class Mesh {
	public:
		enum class Topology {
			Points,
			Lines,
			LineStrip,
			Triangles,
			TriangleStrip,
			TriangleFan
		};

		Mesh(Topology pt = Topology::Triangles) :
			topology(pt) {}
		virtual ~Mesh() {}

		uint32_t addVertex(const Vertex& vertex) {
			uint32_t i = vertices.size();
			vertices.push_back(vertex);
			return i;
		}

		void setTopology(Topology pt) {
			topology = pt;
		}

		Topology getTopology() const {
			return topology;
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
		Topology topology;
		std::vector<Vertex> vertices;
	};
}

#endif
