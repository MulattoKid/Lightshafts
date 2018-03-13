#pragma once

#include "include/GL/glew.h"
#include <string>

struct Shader
{
	void Init(const std::string& vertexShader, const std::string& fragmentShader);
	void Bind();
	void Destroy();

	//Variables
	GLuint shader_program;
};

