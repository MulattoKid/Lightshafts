#pragma once

#include "glm/glm.hpp"

struct Light
{
	glm::vec3 position;
	glm::vec3 dir;
	float cutoff;
	glm::vec3 color;
	glm::mat4 view;
	glm::mat4 vp;
};