#include <Engine/MeshGeneration.h>
#include <Engine/Entity.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <fstream>
#include "GLContext.h"
#include "GLWindow.h"

using namespace std;
using namespace Engine;

int main(int argc, char** argv) {
	try {
		GLContext context;
		Window& window = context.createWindow(1024, 768, 0);
		Renderer& renderer = window.getRenderer();

		Mesh cubeMesh = generateCube();
		IndexedMesh sphereMesh = generateSphere(5);

		Node cube1;
		Node cube2(&cube1);
		Node sphere(&cube1);
		cube2.translate(2.f, 0.f, 0.f);
		cube2.rotate(glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));
		sphere.translate(0.f, 2.f, 0.f);
		cube1.update();

		Material red;
		red.setColor(1.f, 0.f, 0.f, 1.f);
		Material green;
		green.setColor(0.f, 1.f, 0.f, 1.f);
		Material blue;
		blue.setColor(0.f, 0.f, 1.f, 1.f);

		vector<Entity> entities {
			Entity(&cubeMesh, &cube1, &red),
			Entity(&cubeMesh, &cube2, &green),
			Entity(&sphereMesh, &sphere, &blue)
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
		while (!window.shouldClose()) {
			renderer.render(camera, entities);
			glfwPollEvents();

			if (window.isCursorHidden()) {
				glm::vec3 cameraDirection = quatTransform(
					cameraNode.getOrientation(), glm::vec3(0.f, 0.f, -1.f));
				glm::vec3 cameraRight = quatTransform(
					cameraNode.getOrientation(), glm::vec3(1.f, 0.f, 0.f));

				double seconds = glfwGetTime();
				float delta = seconds - time;
				time = seconds;

				glm::vec3 movement;
				bool moved = false;
				bool keyW = window.getKey(Key::W) == KeyAction::Press;
				bool keyA = window.getKey(Key::A) == KeyAction::Press;
				bool keyS = window.getKey(Key::S) == KeyAction::Press;
				bool keyD = window.getKey(Key::D) == KeyAction::Press;
				bool keyQ = window.getKey(Key::Q) == KeyAction::Press;
				bool keyE = window.getKey(Key::E) == KeyAction::Press;
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

				glm::vec2 motion = window.mouseMotion();
				yaw -= motion.x * 0.005f;
				pitch -= motion.y * 0.005f;

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

