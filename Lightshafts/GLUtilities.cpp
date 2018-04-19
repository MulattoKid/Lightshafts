#include "GLUtilities.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <string>

float Lerp(float a, float b, float p)
{
	float range = fabs(a - b);
	return a + (range * p);
}

void LoadTexture(const std::string& file, const int req_comp, bool use_mipmaps, GLint min_filter, GLint mag_filter, GLint wrap_s, GLint wrap_t, GLuint* id)
{
	//Set stb to flip image
	stbi_set_flip_vertically_on_load(1);
	int x, y, comp;
	unsigned char* img_data = stbi_load(file.c_str(), &x, &y, &comp, req_comp);

	glGenTextures(1, id);
	glBindTexture(GL_TEXTURE_2D, *id);
	if (req_comp == 1)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, x, y, 0, GL_RED, GL_UNSIGNED_BYTE, img_data);
	}
	else if (req_comp == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data);
	}
	else if (req_comp == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
	}

	if (use_mipmaps)
	{
		assert(min_filter == GL_NEAREST_MIPMAP_NEAREST || min_filter == GL_NEAREST_MIPMAP_LINEAR ||
			   min_filter == GL_LINEAR_MIPMAP_NEAREST  || min_filter == GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		assert(min_filter == GL_NEAREST || min_filter == GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(img_data);
}

void LoadGeometry(GLuint* vao, const GLfloat* vertex_buffer, const int num_vb_items, const GLuint* index_buffer, const int num_ib_items, GLuint* ibo, VertexDataLayout data_type)
{
	//Setup VAO
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);
	//Setup VBO
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vb_items * sizeof(GLfloat), vertex_buffer, GL_STATIC_DRAW);
	switch (data_type)
	{
	case VERTEX:
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		break;
	case VERTEX_NORMAL:
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
		break;
	case VERTEX_UV:
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
		break;
	case VERTEX_NORMAL_UV:
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
		break;
	case VERTEX_NORMAL_COLOR:
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
		break;
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenBuffers(1, ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_ib_items * sizeof(GLuint), index_buffer, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}