#pragma once

#include "Camera.h"
#include <chrono>
#include "include/GL/glew.h"
#include "include/glm/glm.hpp"
#include "Light.h"
#include "OBJLoader.h"
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

	//Camera
	Camera camera;
	glm::mat4 camera_perspective_matrix;

	//Light
	Light lights[3];
	glm::mat4 light_perspective_matrix;

	//Other data
	float total_time, frame_time;
	bool lightshaft_basic;

	//Shaders
	Shader shader_shadow, shader_gbuffer, shader_lightshaft, shader_compute_scattering, shader_add_scattering, shader_particle;
	//UBO
	GLuint ubo;
	GLuint u_shader_shadow_ubo, u_shader_gbuffer_ubo, u_shader_lightshaft_ubo, u_shader_compute_scattering_ubo, u_shader_add_scattering_ubo, u_shader_particle_ubo;
	//Uniforms
	GLuint u_shadow_model_matrix;
	GLuint u_gbuffer_texture_shadow, u_gbuffer_model_matrix;
	GLuint u_lightshaft_texture_shadow, u_lightshaft_texture_color, u_lightshaft_texture_position;
	GLuint u_compute_scattering_texture_shadow, u_compute_scattering_texture_position, u_compute_scattering_texture_noise;
	GLuint u_add_scattering_texture_color, u_add_scattering_texture_scattering;
	GLuint u_particle_texture_shadow, u_particle_texture_scattering;

	//Meshes
	GLuint surr_cube_vao, surr_cube_ibo;
	GLuint blocking_plane_light_vao, blocking_plane_light_ibo;
	GLuint blocking_plane_camera_vao, blocking_plane_camera_ibo;
	GLuint cube_vao, cube_ibo;
	GLuint alucy_vao, alucy_ibo; Mesh alucy;
	GLuint particle_vao, particle_ibo, particle_centers_vbo; static const unsigned int num_particles_x = 30, num_particles_y = 18, num_particles_z = 30;

	//Framebuffers and textures
	GLuint fbo_shadow, texture_shadow;
	GLuint fbo_gbuffer, texture_depth, texture_color, texture_position;
	GLuint fbo_scattering, texture_scattering;
	//Noise textures
	GLuint texture_blue_noise, texture_perlin_noise;

	//Test quad
	Shader shader_quad;
	GLuint quad_vao, quad_ibo;
	GLuint u_shader_quad_ubo;
};