#include "Camera.h"
#include "include/sdl/SDL.h"

Camera::Camera() : position(0.0f, 0.0f, 2.0f), view_direction(0.0f, 0.0f, -1.0f), up(0.0f, 1.0f, 0.0f), movement_speed(0.25f)
{}

Camera::Camera(glm::vec3 position, glm::vec3 view_direction, glm::vec3 up, float movement_speed, bool look_at_center)
{
	this->position = position;
	if (look_at_center)
	{
		this->view_direction = glm::normalize(glm::vec3(0.0f) - position);
	}
	else
	{
		this->view_direction = glm::normalize(view_direction);
	}
	this->up = glm::normalize(up);
	this->movement_speed = movement_speed;
}

void Camera::UpdatePosition(SDL_Keycode key)
{
	switch (key)
	{
	case SDLK_w:
		position += movement_speed * view_direction;
		break;
	case SDLK_s:
		position += -movement_speed * view_direction;
		break;
	case SDLK_a:
		position += -movement_speed * glm::cross(view_direction, up);
		break;
	case SDLK_d:
		position += movement_speed * glm::cross(view_direction, up);
		break;
	case SDLK_r:
		position += movement_speed * glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case SDLK_f:
		position += -movement_speed * glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	}
}

void Camera::UpdateViewDirection(SDL_Event e)
{
	if (rotate)
	{
		glm::mat4 rotator = glm::rotate(glm::radians(e.motion.xrel * movement_speed), glm::vec3(0.0f, -1.0f, 0.0f)) * //X rotation
			glm::rotate(glm::radians(e.motion.yrel * movement_speed), glm::vec3(-1.0f, 0.0f, 0.0f)); //Y rotation

		view_direction = glm::mat3(rotator) * view_direction;
	}
}

glm::mat4 Camera::WorldToViewMatrix()
{
	return glm::lookAt(position, position + view_direction, up);
}