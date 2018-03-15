#pragma once

#include "include/glm/glm.hpp"

struct Light
{
	glm::vec3 position;
	glm::vec3 color;
	glm::mat4 vp;
};