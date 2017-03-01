#ifndef ENGINE_MESHGENERATION_H
#define ENGINE_MESHGENERATION_H

#include <Engine/IndexedMesh.h>
#include <Engine/Mesh.h>

namespace Engine {
	Mesh generateCube();
	IndexedMesh generateSphere(unsigned subdivisions);
}

#endif
