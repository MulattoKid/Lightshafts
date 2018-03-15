/*#pragma once

#include "include/glm/glm.hpp"
#include <string>

struct Triangle
{
	glm::vec3 v0, v1, v2;
	glm::vec2 uv0, uv1, uv2;
	glm::vec3 normal0, normal1, normal2;
	glm::vec3 normal;
};

struct Mesh
{
	Triangle* triangles;
	unsigned int num_triangles;
	unsigned int* indices;
	unsigned int num_indices;
	bool has_uvs;
	bool has_normals;
};

void LoadOBJ(const std::string& file, Mesh* mesh);*/