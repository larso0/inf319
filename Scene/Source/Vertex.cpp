#include <Scene/Vertex.h>

namespace Scene {
	const size_t Vertex::STRIDE = sizeof(Vertex);
	const size_t Vertex::POSITION_OFFSET = 0;
	const size_t Vertex::NORMAL_OFFSET = sizeof(glm::vec3);
	const size_t Vertex::TEXTURE_COORDINATE_OFFSET = 2 * sizeof(glm::vec3);
}
