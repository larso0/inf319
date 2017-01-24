#ifndef SCENE_GEOMETRYGENERATION_H
#define SCENE_GEOMETRYGENERATION_H

#include "Geometry.h"
#include "ElementGeometry.h"

namespace Scene {
	Geometry generateCube();
	ElementGeometry generateSphere(unsigned subdivisions);
}

#endif
