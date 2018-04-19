#pragma once

#include "glm/glm.hpp"
#include <vector>

struct Mesh
{
	unsigned int num_vertices;
	std::vector<float> vertex_buffer;
	unsigned int num_indices;
	std::vector<unsigned int> index_buffer;
    bool has_uvs;
    bool has_normals;
};