#include <Scene/GeometryGeneration.h>

using glm::vec3;
using glm::vec2;

namespace Scene {
	Geometry generateCube() {
		Geometry g;
		g.addVertex({ vec3(-0.5f, -0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(0.f, 0.f) });
		g.addVertex({ vec3(0.5f, -0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(1.f, 0.f) });
		g.addVertex({ vec3(0.5f, 0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(0.5f, 0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(-0.5f, 0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(0.f, 1.f) });
		g.addVertex({ vec3(-0.5f, -0.5f, 0.5f), vec3(0.f, 0.f, 1.f), vec2(0.f, 0.f) });
		g.addVertex({ vec3(-0.5f, 0.5f, 0.5f), vec3(0.f, 1.f, 0.f), vec2(0.f, 0.f) });
		g.addVertex({ vec3(0.5f, 0.5f, 0.5f), vec3(0.f, 1.f, 0.f), vec2(1.f, 0.f) });
		g.addVertex({ vec3(0.5f, 0.5f, -0.5f), vec3(0.f, 1.f, 0.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(0.5f, 0.5f, -0.5f), vec3(0.f, 1.f, 0.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(-0.5f, 0.5f, -0.5f), vec3(0.f, 1.f, 0.f), vec2(0.f, 1.f) });
		g.addVertex({ vec3(-0.5f, 0.5f, 0.5f), vec3(0.f, 1.f, 0.f), vec2(0.f, 0.f) });
		g.addVertex({ vec3(-0.5f, 0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(0.f, 0.f) });
		g.addVertex({ vec3(0.5f, 0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(1.f, 0.f) });
		g.addVertex({ vec3(0.5f, -0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(0.5f, -0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(-0.5f, -0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(0.f, 1.f) });
		g.addVertex({ vec3(-0.5f, 0.5f, -0.5f), vec3(0.f, 0.f, -1.f), vec2(0.f, 0.f) });
		g.addVertex({ vec3(-0.5f, -0.5f, -0.5f), vec3(-1.f, 0.f, 0.f), vec2(0.f, 0.f) });
		g.addVertex({ vec3(-0.5f, -0.5f, 0.5f), vec3(-1.f, 0.f, 0.f), vec2(1.f, 0.f) });
		g.addVertex({ vec3(-0.5f, 0.5f, 0.5f), vec3(-1.f, 0.f, 0.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(-0.5f, 0.5f, 0.5f), vec3(-1.f, 0.f, 0.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(-0.5f, 0.5f, -0.5f), vec3(-1.f, 0.f, 0.f), vec2(0.f, 1.f) });
		g.addVertex({ vec3(-0.5f, -0.5f, -0.5f), vec3(-1.f, 0.f, 0.f), vec2(0.f, 0.f) });
		g.addVertex({ vec3(-0.5f, -0.5f, 0.5f), vec3(0.f, -1.f, 0.f), vec2(0.f, 0.f) });
		g.addVertex({ vec3(0.5f, -0.5f, -0.5f), vec3(0.f, -1.f, 0.f), vec2(1.f, 0.f) });
		g.addVertex({ vec3(0.5f, -0.5f, 0.5f), vec3(0.f, -1.f, 0.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(-0.5f, -0.5f, -0.5f), vec3(0.f, -1.f, 0.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(0.5f, -0.5f, -0.5f), vec3(0.f, -1.f, 0.f), vec2(0.f, 1.f) });
		g.addVertex({ vec3(-0.5f, -0.5f, 0.5f), vec3(0.f, -1.f, 0.f), vec2(0.f, 0.f) });
		g.addVertex({ vec3(0.5f, -0.5f, 0.5f), vec3(1.f, 0.f, 0.f), vec2(0.f, 0.f) });
		g.addVertex({ vec3(0.5f, -0.5f, -0.5f), vec3(1.f, 0.f, 0.f), vec2(1.f, 0.f) });
		g.addVertex({ vec3(0.5f, 0.5f, -0.5f), vec3(1.f, 0.f, 0.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(0.5f, 0.5f, -0.5f), vec3(1.f, 0.f, 0.f), vec2(1.f, 1.f) });
		g.addVertex({ vec3(0.5f, 0.5f, 0.5f), vec3(1.f, 0.f, 0.f), vec2(0.f, 1.f) });
		g.addVertex({ vec3(0.5f, -0.5f, 0.5f), vec3(1.f, 0.f, 0.f), vec2(0.f, 0.f) });
		return g;
	}

	static glm::vec2 sphereUv(glm::vec3& p) {
		float twoPi = 2 * M_PI;
		glm::vec2 uv;
		uv.x = 0.5f - ((float)atan2(p.z, p.x) / twoPi);
		uv.y = 0.5f - 2.0f * ((float)asin(p.y) / twoPi);
		return uv;
	}


	static uint32_t subVertex(ElementGeometry& g, uint32_t a, uint32_t b) {
		auto& vertices = g.getVertices();
		uint32_t index = vertices.size();
		vec3 pos = glm::normalize(glm::mix(vertices[a].position, vertices[b].position, 0.5f));
		g.addVertex({ pos, pos, sphereUv(pos) });
		return index;
	}

	static void subdivide(ElementGeometry& g, uint32_t a, uint32_t b, uint32_t c, unsigned level) {
		if (level > 0) {
			uint32_t ab = subVertex(g, a, b);
			uint32_t ac = subVertex(g, a, c);
			uint32_t bc = subVertex(g, b, c);
			level--;
			subdivide(g, a, ab, ac, level);
			subdivide(g, ab, b, bc, level);
			subdivide(g, bc, c, ac, level);
			subdivide(g, ab, bc, ac, level);
		} else {
			g.addFace(a, b, c);
		}
	}

	ElementGeometry generateSphere(unsigned subdivisions) {
		ElementGeometry g;

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
		g.addVertex({ va, va, sphereUv(va) });
		uint32_t b = 1;
		g.addVertex({ vb, vb, sphereUv(vb) });
		uint32_t c = 2;
		g.addVertex({ vc, vc, sphereUv(vc) });
		uint32_t d = 3;
		g.addVertex({ vd, vd, sphereUv(vd) });
		uint32_t e = 4;
		g.addVertex({ ve, ve, sphereUv(ve) });
		uint32_t f = 5;
		g.addVertex({ vf, vf, sphereUv(vf) });

		subdivide(g, a, b, e, subdivisions);
		subdivide(g, b, c, e, subdivisions);
		subdivide(g, c, d, e, subdivisions);
		subdivide(g, d, a, e, subdivisions);
		subdivide(g, b, a, f, subdivisions);
		subdivide(g, c, b, f, subdivisions);
		subdivide(g, d, c, f, subdivisions);
		subdivide(g, a, d, f, subdivisions);

		return g;
	}
}
