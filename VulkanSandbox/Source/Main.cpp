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

		float yaw = 0.f, pitch = 0.f;
		double time = glfwGetTime();
		while (!glfwWindowShouldClose(window.handle)) {
			renderer.render(camera, entities);
			glfwPollEvents();

			if (window.mouse.hidden) {
				glm::vec3 cameraDirection = quatTransform(
					cameraNode.getOrientation(), glm::vec3(0.f, 0.f, -1.f));
				glm::vec3 cameraRight = quatTransform(
					cameraNode.getOrientation(), glm::vec3(1.f, 0.f, 0.f));

				double seconds = glfwGetTime();
				float delta = seconds - time;
				time = seconds;

				glm::vec3 movement;
				bool moved = false;
				bool keyW = glfwGetKey(window.handle, GLFW_KEY_W) == GLFW_PRESS;
				bool keyA = glfwGetKey(window.handle, GLFW_KEY_A) == GLFW_PRESS;
				bool keyS = glfwGetKey(window.handle, GLFW_KEY_S) == GLFW_PRESS;
				bool keyD = glfwGetKey(window.handle, GLFW_KEY_D) == GLFW_PRESS;
				bool keyQ = glfwGetKey(window.handle, GLFW_KEY_Q) == GLFW_PRESS;
				bool keyE = glfwGetKey(window.handle, GLFW_KEY_E) == GLFW_PRESS;
				if (keyW && !keyS) {
					movement += cameraDirection;
					moved = true;
				} else if (keyS && !keyW) {
					movement -= cameraDirection;
					moved = true;
				}
				if (keyA && !keyD) {
					movement -= cameraRight;
					moved = true;
				} else if (keyD && !keyA) {
					movement += cameraRight;
					moved = true;
				}
				if (keyQ && !keyE) {
					movement -= glm::vec3(0.f, 1.f, 0.f);
					moved = true;
				} else if (keyE && !keyQ) {
					movement += glm::vec3(0.f, 1.f, 0.f);
					moved = true;
				}

				if (moved) {
					movement = glm::normalize(movement) * delta
						* 2.f;
					cameraNode.translate(movement);
				}

				yaw -= window.mouse.motion.x * window.mouse.sensitivity;
				pitch -= window.mouse.motion.y * window.mouse.sensitivity;
				window.mouse.motion = glm::vec2();

				cameraNode.setRotation(yaw, glm::vec3(0.f, 1.f, 0.f));
				cameraNode.rotate(pitch, glm::vec3(1.f, 0.f, 0.f));
				cameraNode.update();
				camera.update();
			}
		}
	} catch (const exception& e) {
		cerr << e.what() << endl;
		return 1;
	}

	return 0;
}
