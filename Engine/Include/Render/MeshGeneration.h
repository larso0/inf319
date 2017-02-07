#ifndef RENDER_MESHGENERATION_H
#define RENDER_MESHGENERATION_H

#include <Render/IndexedMesh.h>
#include <Render/Mesh.h>

namespace Render {
	Mesh generateCube();
	IndexedMesh generateSphere(unsigned subdivisions);
}

#endif
