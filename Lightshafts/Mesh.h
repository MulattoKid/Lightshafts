#pragma once

#include "include/GL/glew.h"
#include "GLUtilities.h"

struct Mesh
{
	VertexDataLayout data_layout;
	float* vertices;
	unsigned int num_vertices;
	unsigned int* indices;
	unsigned int num_indices;
	GLuint vao, ibo;
};