#ifndef SCENE_VERTEX_H
#define SCENE_VERTEX_H

#include "Math.h"

namespace Scene {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 textureCoordinate;
	};
}

#endif
