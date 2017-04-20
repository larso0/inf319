#include <Engine/MeshGeneration.h>
#include <Engine/Entity.h>
#include <iostream>
#include <stdexcept>
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

class KeyHandler : public KeyEventHandler {
public:
	KeyHandler(Window& window) : window(window) {
		window.addEventHandler((EventHandler*)this);
	}

	void keyRelease(Key key, Modifier) override {
		switch (key) {
		case Key::Escape:
			window.toggleCursorHidden();
			break;
		default:
			break;
		};
	}

private:
	Window& window;
};

class PerspectiveHandler : public WindowEventHandler {
public:
	PerspectiveHandler(Camera& camera) :
		camera(camera),
		fieldOfView(60.f),
		aspectRatio(4.f / 3.f),
		clipNear(0.1f),
		clipFar(500.f) {}

	float getFieldOfView() const { return fieldOfView; }
	float getAspectRatio() const { return aspectRatio; }
	float getNear() const { return clipNear; }
	float getFar() const { return clipFar; }

	void setFieldOfView(float fov) { fieldOfView = fov; }
	void setAspectRatio(float ratio) { aspectRatio = ratio; }
	void setNear(float n) { clipNear = n; }
	void setFar(float f) { clipFar = f; }

	void resize(int w, int h) override {
		aspectRatio = w / (float)h;
		camera.setPerspectiveProjection(glm::radians(fieldOfView), aspectRatio,
			clipNear, clipFar);
	}

private:
	Camera& camera;
	float fieldOfView;
	float aspectRatio;
	float clipNear, clipFar;
};

int main(int argc, char** argv) {
	try {
		VulkanContext vkContext;
		Window& window = vkContext.createWindow(1024, 768, 0);
		Renderer& renderer = window.getRenderer();

		KeyHandler keyHandler(window);

		Mesh cubeMesh = generateCube();
		IndexedMesh sphereMesh = generateSphere(3);
		IndexedMesh supriseMesh = loadMesh("../Assets/monkey.obj");
		IndexedMesh terrainMesh = loadMesh("../Assets/terrain.obj");

		Texture texture("../Assets/texture.jpg");

		Material darkGreen;
		darkGreen.setColor(0.f, 0.5f, 0.f, 1.f);
		Material red;
		red.setColor(1.f, 0.f, 0.f, 1.f);
		Material green;
		green.setColor(0.f, 1.f, 0.f, 1.f);
		Material blue;
		blue.setColor(0.f, 0.f, 1.f, 1.f);
		blue.setTexture(&texture);

		Geometry greenTerrain(&terrainMesh, &darkGreen);
		Geometry redCube(&cubeMesh, &red);
		Geometry greenCube(&cubeMesh, &green);
		Geometry blueSphere(&sphereMesh, &blue);
		Geometry redSuprise(&supriseMesh, &red);

		Node terrain;
		Node cube1(&terrain);
		Node cube2(&cube1);
		Node sphere(&cube1);
		Node suprise(&cube2);
		cube1.translate(0.f, 30.f, 0.f);
		cube2.translate(2.f, 0.f, 0.f);
		cube2.rotate(glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));
		sphere.translate(0.f, 2.f, 0.f);
		suprise.translate(0.f, 5.f, 0.f);
		terrain.update();

		Entity e0(&terrain, &greenTerrain),
			   e1(&cube1, &redCube),
			   e2(&cube2, &greenCube),
			   e3(&sphere, &blueSphere),
			   e4(&suprise, &redSuprise);
		e2.setScale(0.2f, 2.f, 0.2f);

		LightSource light;
		light.setDirection(-0.5f, 1.f, 0.f);
		light.setColor(0.9f, 0.9f, 0.7f);

		Node cameraNode;
		cameraNode.translate(0.f, 30.f, 3.f);
		cameraNode.update();

		Camera camera(&cameraNode);
		PerspectiveHandler perspectiveHandler(camera);
		window.addEventHandler((EventHandler*)&perspectiveHandler);
		perspectiveHandler.resize(window.getWidth(), window.getHeight());
		camera.update();

		renderer.addEntity(&e0);
		renderer.addEntity(&e1);
		renderer.addEntity(&e2);
		renderer.addEntity(&e3);
		renderer.addEntity(&e4);
		renderer.addLightSource(&light);
		renderer.setCamera(&camera);

		float yaw = 0.f, pitch = 0.f;
		double time = glfwGetTime();
		while (!window.shouldClose()) {
			renderer.render();
			window.handleEvents();

			double seconds = glfwGetTime();
			float delta = seconds - time;
			time = seconds;

			if (window.isCursorHidden()) {
				glm::vec3 cameraDirection = quatTransform(
					cameraNode.getOrientation(), glm::vec3(0.f, 0.f, -1.f));
				glm::vec3 cameraRight = quatTransform(
					cameraNode.getOrientation(), glm::vec3(1.f, 0.f, 0.f));

				glm::vec3 movement;
				bool moved = false;
				bool keyW = window.getKey(Key::W) == KeyAction::Press;
				bool keyA = window.getKey(Key::A) == KeyAction::Press;
				bool keyS = window.getKey(Key::S) == KeyAction::Press;
				bool keyD = window.getKey(Key::D) == KeyAction::Press;
				bool keyQ = window.getKey(Key::Q) == KeyAction::Press;
				bool keyE = window.getKey(Key::E) == KeyAction::Press;
				bool keyH = window.getKey(Key::H) == KeyAction::Press;
				bool keyJ = window.getKey(Key::J) == KeyAction::Press;
				bool keyK = window.getKey(Key::K) == KeyAction::Press;
				bool keyL = window.getKey(Key::L) == KeyAction::Press;
				bool keyU = window.getKey(Key::U) == KeyAction::Press;
				bool keyI = window.getKey(Key::I) == KeyAction::Press;
				if (keyW && !keyS) {
					movement += cameraDirection;
					moved = true;
				}
				else if (keyS && !keyW) {
					movement -= cameraDirection;
					moved = true;
				}
				if (keyA && !keyD) {
					movement -= cameraRight;
					moved = true;
				}
				else if (keyD && !keyA) {
					movement += cameraRight;
					moved = true;
				}
				if (keyQ && !keyE) {
					movement -= glm::vec3(0.f, 1.f, 0.f);
					moved = true;
				}
				else if (keyE && !keyQ) {
					movement += glm::vec3(0.f, 1.f, 0.f);
					moved = true;
				}

				if (moved) {
					movement = glm::normalize(movement) * delta
						* 2.f;
					cameraNode.translate(movement);
				}

				moved = false;
				movement = glm::vec3();
				if (keyH && !keyL) {
					movement.x = 1;
					moved = true;
				}
				else if (keyL && !keyH) {
					movement.x = -1;
					moved = true;
				}
				if (keyJ && !keyK) {
					movement.y = -1;
					moved = true;
				}
				else if (keyK && !keyJ) {
					movement.y = 1;
					moved = true;
				}
				if (keyU && !keyI) {
					suprise.rotate(-delta, glm::vec3(0.f, 1.f, 0.f));
				}
				else if (keyI && !keyU) {
					suprise.rotate(delta, glm::vec3(0.f, 1.f, 0.f));
				}

				if (moved) {
					movement = glm::normalize(movement) * delta;
					suprise.translate(movement);
				}
				suprise.update();

				glm::vec2 motion = window.mouseMotion();
				yaw -= motion.x * 0.005f;
				pitch -= motion.y * 0.005f;

				cameraNode.setRotation(yaw, glm::vec3(0.f, 1.f, 0.f));
				cameraNode.rotate(pitch, glm::vec3(1.f, 0.f, 0.f));
				cameraNode.update();
			}
			camera.update();
		}
	} catch (const exception& e) {
		cerr << e.what() << endl;
		return 1;
	}

	return 0;
}
