#ifndef ENGINE_MESHGENERATION_H
#define ENGINE_MESHGENERATION_H

#include <Engine/IndexedMesh.h>
#include <Engine/Mesh.h>
#include <string>

namespace Engine {
	Mesh generateCube();
	IndexedMesh generateSphere(unsigned subdivisions);
	IndexedMesh loadMesh(const std::string& filePath);
}

#endif
