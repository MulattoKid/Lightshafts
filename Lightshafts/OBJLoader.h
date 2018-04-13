#pragma once

#include "include\glm\glm.hpp"
#include <string>
#include "Mesh.h"

int LoadOBJ(const std::string& file, Mesh* mesh, glm::vec3 color);