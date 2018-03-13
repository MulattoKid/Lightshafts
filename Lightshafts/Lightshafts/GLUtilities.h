#pragma once

#include "include/GL/glew.h"
#include "OBJLoader.h"
#include <string>

struct UBOData
{
	float vp[16];
	//float light_vp[16];
	//float cam_position[4];
	//float light_position[4];
};

enum VertexDataLayout
{
	VERTEX, VERTEX_NORMAL, VERTEX_UV, VERTEX_NORMAL_UV
};

void LoadTexture(const std::string& file, const int req_comp, bool use_mipmaps, GLint min_filter, GLint mag_filter, GLint wrap_s, GLint wrap_t, GLuint* id);
void LoadGeometry(GLuint* vao, const GLfloat* vertex_buffer, const int num_vb_items, const GLuint* index_buffer, const int num_ib_items, GLuint* ibo, VertexDataLayout data_type); //Assuming vertex data comes in ONE buffer