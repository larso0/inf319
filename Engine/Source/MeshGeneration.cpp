#include <Engine/MeshGeneration.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <iostream>
#include <stdexcept>

using namespace std;
using glm::vec3;
using glm::vec2;

namespace Engine {
	Mesh generateCube() {
		Mesh m;
		m.addVertex({ vec3(-0.5f, -0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(0.f, 0.f) });
		m.addVertex({ vec3(0.5f, -0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(1.f, 0.f) });
		m.addVertex({ vec3(0.5f, 0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(0.5f, 0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(-0.5f, 0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(0.f, 1.f) });
		m.addVertex({ vec3(-0.5f, -0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(0.f, 0.f) });
		m.addVertex({ vec3(-0.5f, 0.5f, 0.5f), vec3(0.f, 1.f, 0.f), vec2(0.f, 0.f) });
		m.addVertex({ vec3(0.5f, 0.5f, 0.5f), vec3(0.f, 1.f, 0.f), vec2(1.f, 0.f) });
		m.addVertex({ vec3(0.5f, 0.5f, -0.5f), vec3(0.f, 1.f, 0.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(0.5f, 0.5f, -0.5f), vec3(0.f, 1.f, 0.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(-0.5f, 0.5f, -0.5f), vec3(0.f, 1.f, 0.f), vec2(0.f, 1.f) });
		m.addVertex({ vec3(-0.5f, 0.5f, 0.5f), vec3(0.f, 1.f, 0.f), vec2(0.f, 0.f) });
		m.addVertex({ vec3(-0.5f, 0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(0.f, 0.f) });
		m.addVertex({ vec3(0.5f, 0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(1.f, 0.f) });
		m.addVertex({ vec3(0.5f, -0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(0.5f, -0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(-0.5f, -0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(0.f, 1.f) });
		m.addVertex({ vec3(-0.5f, 0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(0.f, 0.f) });
		m.addVertex({ vec3(-0.5f, -0.5f, -0.5f), vec3(-1.f, 0.f, 0.f), vec2(0.f, 0.f) });
		m.addVertex({ vec3(-0.5f, -0.5f, 0.5f), vec3(-1.f, 0.f, 0.f), vec2(1.f, 0.f) });
		m.addVertex({ vec3(-0.5f, 0.5f, 0.5f), vec3(-1.f, 0.f, 0.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(-0.5f, 0.5f, 0.5f), vec3(-1.f, 0.f, 0.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(-0.5f, 0.5f, -0.5f), vec3(-1.f, 0.f, 0.f), vec2(0.f, 1.f) });
		m.addVertex({ vec3(-0.5f, -0.5f, -0.5f), vec3(-1.f, 0.f, 0.f), vec2(0.f, 0.f) });
		m.addVertex({ vec3(-0.5f, -0.5f, 0.5f), vec3(0.f, -1.f, 0.f), vec2(0.f, 0.f) });
		m.addVertex({ vec3(0.5f, -0.5f, -0.5f), vec3(0.f, -1.f, 0.f), vec2(1.f, 0.f) });
		m.addVertex({ vec3(0.5f, -0.5f, 0.5f), vec3(0.f, -1.f, 0.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(-0.5f, -0.5f, -0.5f), vec3(0.f, -1.f, 0.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(0.5f, -0.5f, -0.5f), vec3(0.f, -1.f, 0.f), vec2(0.f, 1.f) });
		m.addVertex({ vec3(-0.5f, -0.5f, 0.5f), vec3(0.f, -1.f, 0.f), vec2(0.f, 0.f) });
		m.addVertex({ vec3(0.5f, -0.5f, 0.5f), vec3(1.f, 0.f, 0.f), vec2(0.f, 0.f) });
		m.addVertex({ vec3(0.5f, -0.5f, -0.5f), vec3(1.f, 0.f, 0.f), vec2(1.f, 0.f) });
		m.addVertex({ vec3(0.5f, 0.5f, -0.5f), vec3(1.f, 0.f, 0.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(0.5f, 0.5f, -0.5f), vec3(1.f, 0.f, 0.f), vec2(1.f, 1.f) });
		m.addVertex({ vec3(0.5f, 0.5f, 0.5f), vec3(1.f, 0.f, 0.f), vec2(0.f, 1.f) });
		m.addVertex({ vec3(0.5f, -0.5f, 0.5f), vec3(1.f, 0.f, 0.f), vec2(0.f, 0.f) });
		return m;
	}

	static glm::vec2 sphereUv(glm::vec3& p) {
		float twoPi = 2 * M_PI;
		glm::vec2 uv;
		uv.x = 0.5f - ((float)atan2(p.z, p.x) / twoPi);
		uv.y = 0.5f - 2.0f * ((float)asin(p.y) / twoPi);
		return uv;
	}


	static uint32_t subVertex(IndexedMesh& m, uint32_t a, uint32_t b) {
		auto& vertices = m.getVertices();
		uint32_t index = vertices.size();
		vec3 pos = glm::normalize(glm::mix(vertices[a].position, vertices[b].position, 0.5f));
		m.addVertex({ pos, pos, sphereUv(pos) });
		return index;
	}

	static void subdivide(IndexedMesh& m, uint32_t a, uint32_t b, uint32_t c, unsigned level) {
		if (level > 0) {
			uint32_t ab = subVertex(m, a, b);
			uint32_t ac = subVertex(m, a, c);
			uint32_t bc = subVertex(m, b, c);
			level--;
			subdivide(m, a, ab, ac, level);
			subdivide(m, ab, b, bc, level);
			subdivide(m, bc, c, ac, level);
			subdivide(m, ab, bc, ac, level);
		} else {
			m.addFace(a, b, c);
		}
	}

	IndexedMesh generateSphere(unsigned subdivisions) {
		IndexedMesh m;

		vec3
			va(-1.f, 0.f, 1.f),
			vb(1.f, 0.f, 1.f),
			vc(1.f, 0.f, -1.f),
			vd(-1.f, 0.f, -1.f),
			ve(0.f, 1.f, 0.f),
			vf(0.f, -1.f, 0.f);
		va = glm::normalize(va);
		vb = glm::normalize(vb);
		vc = glm::normalize(vc);
		vd = glm::normalize(vd);

		uint32_t a = 0;
		m.addVertex({ va, va, sphereUv(va) });
		uint32_t b = 1;
		m.addVertex({ vb, vb, sphereUv(vb) });
		uint32_t c = 2;
		m.addVertex({ vc, vc, sphereUv(vc) });
		uint32_t d = 3;
		m.addVertex({ vd, vd, sphereUv(vd) });
		uint32_t e = 4;
		m.addVertex({ ve, ve, sphereUv(ve) });
		uint32_t f = 5;
		m.addVertex({ vf, vf, sphereUv(vf) });

		subdivide(m, a, b, e, subdivisions);
		subdivide(m, b, c, e, subdivisions);
		subdivide(m, c, d, e, subdivisions);
		subdivide(m, d, a, e, subdivisions);
		subdivide(m, b, a, f, subdivisions);
		subdivide(m, c, b, f, subdivisions);
		subdivide(m, d, c, f, subdivisions);
		subdivide(m, a, d, f, subdivisions);

		return m;
	}

	Mesh loadMesh(const string& filePath) {
		tinyobj::attrib_t attrib;
		vector<tinyobj::shape_t> shapes;
		vector<tinyobj::material_t> materials;
		string err;
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
			filePath.c_str())) {
			throw runtime_error(err);
		}

		if (!err.empty()) {
			cerr << err;
		}

		Mesh mesh;

		for (const tinyobj::shape_t& shape : shapes) {
			for (const tinyobj::index_t& i : shape.mesh.indices) {
				Vertex v;
				v.position.x = attrib.vertices[i.vertex_index*3];
				v.position.y = attrib.vertices[i.vertex_index*3+1];
				v.position.z = attrib.vertices[i.vertex_index*3+2];
				mesh.addVertex(v);
			}
		}

		for (int i = 0; i < mesh.getElementCount();) {
			Vertex& a = mesh.getVertices()[i++];
			Vertex& b = mesh.getVertices()[i++];
			Vertex& c = mesh.getVertices()[i++];

			a.normal = b.normal = c.normal = glm::normalize(
				glm::cross(a.position - c.position, b.position - c.position));
		}

		return mesh;
	}
}
