#ifndef ENGINE_VERTEX_H
#define ENGINE_VERTEX_H

#include <Engine/Math.h>

namespace Engine {
	class Vertex {
	public:
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 textureCoordinate;

		static const size_t Stride;
		static const size_t PositionOffset;
		static const size_t NormalOffset;
		static const size_t TextureCoordinateOffset;
	};
}

#endif
