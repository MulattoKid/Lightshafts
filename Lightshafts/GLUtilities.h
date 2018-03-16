#pragma once

#include "include/GL/glew.h"
#include "OBJLoader.h"
#include <string>

struct UBOData
{
	float viewport[4];
	float camera_vp[16];
	float camera_to_world[16];
	float camera_pos[4];
	float camera_dir[4];
	float light_pos_0[4];
	float light_color_0[4];
	float light_vp_0[16];
};

enum VertexDataLayout
{
	VERTEX, VERTEX_NORMAL, VERTEX_UV, VERTEX_NORMAL_UV
};

void LoadTexture(const std::string& file, const int req_comp, bool use_mipmaps, GLint min_filter, GLint mag_filter, GLint wrap_s, GLint wrap_t, GLuint* id);
void LoadGeometry(GLuint* vao, const GLfloat* vertex_buffer, const int num_vb_items, const GLuint* index_buffer, const int num_ib_items, GLuint* ibo, VertexDataLayout data_type); //Assuming vertex data comes in ONE buffer