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

		Mesh cubeMesh = generateCube();
		IndexedMesh sphereMesh = generateSphere(5);

		Node cube1;
		Node cube2(&cube1);
		Node sphere(&cube1);
		cube2.translate(2.f, 0.f, 0.f);
		cube2.rotate(glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));
		sphere.translate(0.f, 2.f, 0.f);
		cube1.update();

		vector<Entity> entities {
			Entity(&cubeMesh, &cube1),
			Entity(&cubeMesh, &cube2),
			Entity(&sphereMesh, &sphere)
		};
		entities[1].setScale(0.2f, 2.f, 0.2f);

		Node cameraNode;
		cameraNode.translate(0.f, 0.f, 3.f);
		cameraNode.update();

		Camera camera(&cameraNode);
		camera.setPerspectiveProjection(glm::radians(60.f), 4.f / 3.f, 0.1f,
			100.f);
		camera.update();

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
