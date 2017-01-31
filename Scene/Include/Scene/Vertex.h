#ifndef SCENE_VERTEX_H
#define SCENE_VERTEX_H

#include "Math.h"

namespace Scene {
	class Vertex {
	public:
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 textureCoordinate;

		static const size_t STRIDE;
		static const size_t POSITION_OFFSET;
		static const size_t NORMAL_OFFSET;
		static const size_t TEXTURE_COORDINATE_OFFSET;
	};
}

#endif
