#include <Engine/Vertex.h>

namespace Engine {
	const size_t Vertex::Stride = sizeof(Vertex);
	const size_t Vertex::PositionOffset = 0;
	const size_t Vertex::NormalOffset = sizeof(glm::vec3);
	const size_t Vertex::TextureCoordinateOffset = 2 * sizeof(glm::vec3);
}
