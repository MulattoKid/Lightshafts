#pragma once

#include "Camera.h"
#include <chrono>
#include "include/GL/glew.h"
#include "include/glm/glm.hpp"
#include "Light.h"
#include "Mesh.h"
#include "include/sdl/SDL.h"
#include "Shader.h"
#include <string>

struct Window
{
	//Functions
	void Create(const std::string& name, int width, int height);
	void Init();
	void InitLights();
	void Update();
	void CheckForEvents();

	//Key variables
	bool is_closed;
	unsigned int screen_width;
	unsigned int screen_height;
	SDL_Window* window;

	//Camera
	Camera camera;
	glm::mat4 camera_perspective_matrix;

	//Light
	Light lights[3];
	glm::mat4 light_perspective_matrix;

	//Other data
	float total_time, frame_time;

	//Shaders
	GLuint ubo;
	Shader shader_shadow, shader_lightshaft;
	GLuint u_shader_shadow_ubo, u_shader_lightshaft_ubo;
	GLuint u_gbuffer_texture_shadow;

	//Meshes
	GLuint plane_vao, plane_ibo;
	GLuint cube_vao, cube_ibo;

	//Framebuffers and textures
	GLuint fbo_shadow, texture_shadow;

	//Test quad
	Shader shader_quad;
	GLuint quad_vao, quad_ibo;
	GLuint u_shader_quad_ubo;
};