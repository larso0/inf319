#ifndef ENGINE_MESHGENERATION_H
#define ENGINE_MESHGENERATION_H

#include "IndexedMesh.h"
#include "Mesh.h"
#include <string>

namespace Engine {
	Mesh generateCube();
	IndexedMesh generateSphere(unsigned subdivisions);
	IndexedMesh loadMesh(const std::string& filePath);
}

#endif
