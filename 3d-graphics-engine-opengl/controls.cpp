// Controls.cpp: input en timing
#include <SDL3/SDL.h>
extern SDL_Window* gWindow; // window uit main

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.h"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}

// Beginpositie van de camera
glm::vec3 position = glm::vec3(0, 0, 5);
// Camera front, right en up vectors
glm::vec3 front;
glm::vec3 up;
glm::vec3 right;
// Beginhoek horizontaal : naar -Z
float horizontalAngle = 3.14f;
// Beginhoek verticaal : none
float verticalAngle = 0.0f;
// Begin FOV
float initialFoV = 45.0f;

float speed = 3.0f; // 3 /s
float lookSpeed = 1.5f;


void computeMatricesFromInputs(float aspect, float near, float far) {

	static double lastTime = (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();

	// Bereken tijd verschil tussen frames
	double currentTime = (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
	float deltaTime = float(currentTime - lastTime);

	int windowWidth = 1024;
	int windowHeight = 768;
	if (gWindow) {
		SDL_GetWindowSize(gWindow, &windowWidth, &windowHeight);
	}

	// Arrow key input voor rondkijken
	const bool* keyState = SDL_GetKeyboardState(NULL);
	if (keyState[SDL_SCANCODE_LEFT]) {
		horizontalAngle += lookSpeed * deltaTime;
	}
	if (keyState[SDL_SCANCODE_RIGHT]) {
		horizontalAngle -= lookSpeed * deltaTime;
	}
	if (keyState[SDL_SCANCODE_UP]) {
		verticalAngle += lookSpeed * deltaTime;
	}
	if (keyState[SDL_SCANCODE_DOWN]) {
		verticalAngle -= lookSpeed * deltaTime;
	}

	// Richting berekenen
	front = {
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	};

	right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	up = glm::cross(right, front);

	// WASD voor beweging
	if (keyState[SDL_SCANCODE_W]) {
		position += front * deltaTime * speed;
	}
	if (keyState[SDL_SCANCODE_S]) {
		position -= front * deltaTime * speed;
	}
	if (keyState[SDL_SCANCODE_D]) {
		position += right * deltaTime * speed;
	}
	if (keyState[SDL_SCANCODE_A]) {
		position -= right * deltaTime * speed;
	}

	// FOV functie gaan we later doen
	float FoV = initialFoV;// - 5 * someSDLscrollwheelFunction();

	// Projection matrix : FOV, 4:3 ratio, range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), aspect, near, far);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is hier
		position + front, // kijkt hiernaartoe
		up                  // Head is omhoog
	);

	lastTime = currentTime;
}

glm::vec3 getCameraPosition() {
	return position;
}

glm::vec3 getCameraFront() {
	return front;
}

glm::vec3 getCameraUp() {
	return up;
}

glm::vec3 getCameraRight() {
	return right;
}

float getFOV() {
	return initialFoV;
}