#include <Engine/MeshGeneration.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <iostream>
#include <stdexcept>
#include <map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
		float twoPi = (float)(2 * M_PI);
		glm::vec2 uv;
		uv.x = 0.5f - ((float)atan2(p.z, p.x) / twoPi);
		uv.y = 0.5f - 2.0f * ((float)asin(p.y) / twoPi);
		return uv;
	}


	static uint32_t subVertex(IndexedMesh& m, uint32_t a, uint32_t b) {
		auto& vertices = m.getVertices();
		vec3 pos = glm::normalize(glm::mix(vertices[a].position, vertices[b].position, 0.5f));
		return m.addVertex({ pos, pos, sphereUv(pos) });;
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

	void repairTextureWrapSeam(vector<Vertex>& vertices, vector<uint32_t>& indices) {
		vector<uint32_t> newIndices;
		map<uint32_t, uint32_t> correctionList;
		for (uint32_t i = 0; i < indices.size(); i += 3) {
			vec3 v0 = vec3(vertices[indices[i + 0]].textureCoordinate, 0);
			vec3 v1 = vec3(vertices[indices[i + 1]].textureCoordinate, 0);
			vec3 v2 = vec3(vertices[indices[i + 2]].textureCoordinate, 0);

			vec3 cross = glm::cross(v0 - v1, v2 - v1);

			if (cross.z <= 0) {
				for (uint32_t j = i; j < i + 3; j++) {
					uint32_t index = indices[j];
					Vertex vertex = vertices[index];
					if (vertex.textureCoordinate.x >= 0.9f) {
						auto found = correctionList.find(index);
						if (found != correctionList.end()) {
							newIndices.push_back(found->second);
						} else {
							vec2 texCoord = vertex.textureCoordinate;
							texCoord.x -= 1;
							vertex.textureCoordinate = texCoord;
							vertices.push_back(vertex);
							uint32_t correctedVertexIndex = (uint32_t)(vertices.size() - 1);
							correctionList[index] = correctedVertexIndex;
							newIndices.push_back(correctedVertexIndex);
						}
					} else {
						newIndices.push_back(index);
					}

				}
			} else {
				for (uint32_t j = i; j < i + 3; j++) {
					newIndices.push_back(indices[j]);
				}
			}
		}
		indices = newIndices;
	}

	IndexedMesh generateSphere(unsigned subdivisions) {
		IndexedMesh m;

		float t = (1.f + sqrtf(5.f)) / 2.f;

		vec3
			v0(-1.f, t, 0.f),
			v1(1.f, t, 0.f),
			v2(-1.f, -t, 0.f),
			v3(1.f, -t, 0.f),

			v4(0.f, -1.f, t),
			v5(0.f, 1.f, t),
			v6(0.f, -1.f, -t),
			v7(0.f, 1.f, -t),
			
			v8(t, 0.f, -1.f),
			v9(t, 0.f, 1.f),
			v10(-t, 0.f, -1.f),
			v11(-t, 0.f, 1.f);

		v0 = glm::normalize(v0);
		v1 = glm::normalize(v1);
		v2 = glm::normalize(v2);
		v3 = glm::normalize(v3);
		v4 = glm::normalize(v4);
		v5 = glm::normalize(v5);
		v6 = glm::normalize(v6);
		v7 = glm::normalize(v7);
		v8 = glm::normalize(v8);
		v9 = glm::normalize(v9);
		v10 = glm::normalize(v10);
		v11 = glm::normalize(v11);

		m.addVertex({ v0, v0, sphereUv(v0) });
		m.addVertex({ v1, v1, sphereUv(v1) });
		m.addVertex({ v2, v2, sphereUv(v2) });
		m.addVertex({ v3, v3, sphereUv(v3) });
		m.addVertex({ v4, v4, sphereUv(v4) });
		m.addVertex({ v5, v5, sphereUv(v5) });
		m.addVertex({ v6, v6, sphereUv(v6) });
		m.addVertex({ v7, v7, sphereUv(v7) });
		m.addVertex({ v8, v8, sphereUv(v8) });
		m.addVertex({ v9, v9, sphereUv(v9) });
		m.addVertex({ v10, v10, sphereUv(v10) });
		m.addVertex({ v11, v11, sphereUv(v11) });

		subdivide(m, 0, 11, 5, subdivisions);
		subdivide(m, 0, 5, 1, subdivisions);
		subdivide(m, 0, 1, 7, subdivisions);
		subdivide(m, 0, 7, 10, subdivisions);
		subdivide(m, 0, 10, 11, subdivisions);

		subdivide(m, 1, 5, 9, subdivisions);
		subdivide(m, 5, 11, 4, subdivisions);
		subdivide(m, 11, 10, 2, subdivisions);
		subdivide(m, 10, 7, 6, subdivisions);
		subdivide(m, 7, 1, 8, subdivisions);

		subdivide(m, 3, 9, 4, subdivisions);
		subdivide(m, 3, 4, 2, subdivisions);
		subdivide(m, 3, 2, 6, subdivisions);
		subdivide(m, 3, 6, 8, subdivisions);
		subdivide(m, 3, 8, 9, subdivisions);

		subdivide(m, 4, 9, 5, subdivisions);
		subdivide(m, 2, 4, 11, subdivisions);
		subdivide(m, 6, 2, 10, subdivisions);
		subdivide(m, 8, 6, 7, subdivisions);
		subdivide(m, 9, 8, 1, subdivisions);

		//repairTextureWrapSeam(m.getVertices(), m.getIndices()); //currently doesn't matter because texture atlas

		return m;
	}

	IndexedMesh loadMesh(const string& filePath) {
		vector<tinyobj::shape_t> shapes;
		vector<tinyobj::material_t> materials;
		string err;
		tinyobj::LoadObj(shapes, materials, err, filePath.c_str(), nullptr,
			tinyobj::load_flags_t::triangulation
				| tinyobj::load_flags_t::calculate_normals);

		if (!err.empty()) {
			cerr << err;
		}

		IndexedMesh mesh;

		for (const tinyobj::shape_t& shape : shapes) {
			for (int i = 0; i < shape.mesh.positions.size() / 3; i++) {
				Vertex v;
				int pos = i*3;
				v.position.x = shape.mesh.positions[pos];
				v.normal.x = shape.mesh.normals[pos++];
				v.position.y = shape.mesh.positions[pos];
				v.normal.y = shape.mesh.normals[pos++];
				v.position.z = shape.mesh.positions[pos];
				v.normal.z = shape.mesh.normals[pos++];

				if (!shape.mesh.texcoords.empty()) {
					pos = i*2;
					v.textureCoordinate.x = shape.mesh.texcoords[pos++];
					v.textureCoordinate.y = shape.mesh.texcoords[pos];
				}
				mesh.addVertex(v);
			}
			uint32_t offset = (uint32_t)mesh.getElementCount();
			for (uint32_t i : shape.mesh.indices) {
				mesh.addIndex(i + offset);
			}
		}

		return mesh;
	}
}
