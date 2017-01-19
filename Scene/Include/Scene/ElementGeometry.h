#ifndef SCENE_ELEMENTGEOMETRY_H
#define SCENE_ELEMENTGEOMETRY_H

#include "Geometry.h"

namespace Scene {
	class ElementGeometry : public Geometry {
	public:
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

	private:
		std::vector<uint32_t> indices;
	};
}

#endif
