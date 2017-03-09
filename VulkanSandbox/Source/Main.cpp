#include <Engine/MeshGeneration.h>
#include <Engine/Entity.h>
#include <iostream>
#include <stdexcept>
#include "Context.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <fstream>
#include "VulkanContext.h"
#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#include "VulkanPerMesh.h"

using namespace std;
using namespace Engine;

int main(int argc, char** argv) {
	try {
		VulkanContext vkContext;
		VulkanWindow window(vkContext);
		VulkanRenderer renderer(window);

		Mesh cube = generateCube();
		vector<Entity> entities = {
			Entity(&cube, nullptr)
		};
		Camera camera;

		while (!glfwWindowShouldClose(window.handle)) {
			renderer.render(camera, entities);
			glfwWaitEvents();
		}
	} catch (const exception& e) {
		cerr << e.what() << endl;
		return 1;
	}

	return 0;
}
