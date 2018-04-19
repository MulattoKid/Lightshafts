#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "sdl/SDL.h"

struct Camera
{
	//Fucntions
	Camera();
	Camera(glm::vec3 position, glm::vec3 viewDirection, glm::vec3 upVector, float movementSpeed, bool look_at_center);
	void UpdatePosition(SDL_Keycode key);
	void UpdateViewDirection(SDL_Event e);
	glm::mat4 WorldToViewMatrix();

	//Variables
	int rotate = 0;
	glm::vec3 position;
	glm::vec3 view_direction;
	glm::vec3 up;
	float movement_speed;
};

