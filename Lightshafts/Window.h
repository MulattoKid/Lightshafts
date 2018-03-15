#pragma once

#include "Camera.h"
#include <chrono>
#include "include/GL/glew.h"
#include "include/glm/glm.hpp"
#include "Mesh.h"
#include "include/sdl/SDL.h"
#include "Shader.h"
#include <string>

struct Window
{
	//Functions
	void Create(const std::string& name, int width, int height);
	void Init();
	void Update();
	void CheckForEvents();

	//Key variables
	bool is_closed;
	unsigned int screen_width;
	unsigned int screen_height;
	SDL_Window* window;

	//General stuff
	Camera camera;
	glm::mat4 perspective_matrix;
	float total_time, frame_time;

	//Shaders
	GLuint ubo;
	Shader shader_plain_normal;
	GLuint u_shader_plain_normal_ubo;

	//Meshes
	GLuint plane_vao, plane_ibo;
	GLuint cube_vao, cube_ibo;
};