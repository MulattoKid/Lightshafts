#include <cstring>
#include "GLUtilities.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Particle.h"
#include <time.h>
#include "Window.h"

#define NEAR_PLANE 0.1f
#define FAR_PLANE 1000.0f
#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024
#define DEFAULT_FRAMEBUFFER 0

void Window::Create(const std::string& name, int width, int height)
{
	screen_width = width;
	screen_height = height;

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_ShowCursor(SDL_DISABLE); //Hide cursor
	SDL_SetRelativeMouseMode(SDL_TRUE); //Cursor doesn't hit end of screen

	window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL);
	if (window == nullptr)
	{
		printf("Failed to create SDL window");
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (context = nullptr)
	{
		printf("Failed to create SDL context");
	}

	GLenum glError = glewInit();
	if (glError != GLEW_OK)
	{
		printf("Glew was not initialized correctly");
	}

	is_closed = false;
	printf("***  OpenGL Version: %s  ***\n", glGetString(GL_VERSION));
}

void Window::Init()
{
	//Camera
	camera = Camera(glm::vec3(0.0f, 2.0f, 10.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.25f, true);
	camera_perspective_matrix = glm::perspective(glm::radians(70.0f), (float)screen_width / screen_height, NEAR_PLANE, FAR_PLANE);
	//Light
	lights[0].position = glm::vec3(4.0f, 10.0f, 0.0f);
	lights[0].dir = glm::normalize(glm::vec3(0.0f) - lights[0].position); //Always look at center
	lights[0].cutoff = glm::cos(glm::radians(30.0f)); //30 degree cutoff
	lights[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
	lights[0].vp = glm::perspective(70.0f, 1.0f, NEAR_PLANE, FAR_PLANE) * glm::lookAt(lights[0].position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	//Other
	total_time = 0.0f, frame_time = 0.0f;

	//Setup render state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//Shaders
	shader_shadow.Init("assets/shaders/shadow.vert", "assets/shaders/shadow.frag");
	shader_gbuffer.Init("assets/shaders/gbuffer.vert", "assets/shaders/gbuffer.frag");
	shader_lightshaft.Init("assets/shaders/lightshaft.vert", "assets/shaders/lightshaft.frag");
	shader_compute_scattering.Init("assets/shaders/compute_scattering.vert", "assets/shaders/compute_scattering.frag");
	shader_add_scattering.Init("assets/shaders/add_scattering.vert", "assets/shaders/add_scattering.frag");
	shader_particle.Init("assets/shaders/particle.vert", "assets/shaders/particle.frag");

	//UBO
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOData), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
	u_shader_shadow_ubo = glGetUniformBlockIndex(shader_shadow.shader_program, "UBOData");
	glUniformBlockBinding(shader_shadow.shader_program, u_shader_shadow_ubo, 0);
	u_shader_gbuffer_ubo = glGetUniformBlockIndex(shader_gbuffer.shader_program, "UBOData");
	glUniformBlockBinding(shader_gbuffer.shader_program, u_shader_gbuffer_ubo, 0);
	u_shader_lightshaft_ubo = glGetUniformBlockIndex(shader_lightshaft.shader_program, "UBOData");
	glUniformBlockBinding(shader_lightshaft.shader_program, u_shader_lightshaft_ubo, 0);
	u_shader_compute_scattering_ubo = glGetUniformBlockIndex(shader_compute_scattering.shader_program, "UBOData");
	glUniformBlockBinding(shader_compute_scattering.shader_program, u_shader_compute_scattering_ubo, 0);
	u_shader_add_scattering_ubo = glGetUniformBlockIndex(shader_add_scattering.shader_program, "UBOData");
	glUniformBlockBinding(shader_add_scattering.shader_program, u_shader_add_scattering_ubo, 0);
	u_shader_particle_ubo = glGetUniformBlockIndex(shader_particle.shader_program, "UBOData");
	glUniformBlockBinding(shader_particle.shader_program, u_shader_particle_ubo, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);

	//Uniforms
	u_shadow_model_matrix = glGetUniformLocation(shader_shadow.shader_program, "model_matrix");
	u_gbuffer_texture_shadow = glGetUniformLocation(shader_gbuffer.shader_program, "shadow_sampler");
	u_gbuffer_model_matrix = glGetUniformLocation(shader_gbuffer.shader_program, "model_matrix");
	u_lightshaft_texture_shadow = glGetUniformLocation(shader_lightshaft.shader_program, "shadow_sampler");
	u_lightshaft_texture_color = glGetUniformLocation(shader_lightshaft.shader_program, "color_sampler");
	u_lightshaft_texture_position = glGetUniformLocation(shader_lightshaft.shader_program, "position_sampler");
	u_compute_scattering_texture_shadow = glGetUniformLocation(shader_compute_scattering.shader_program, "shadow_sampler");
	u_compute_scattering_texture_position = glGetUniformLocation(shader_compute_scattering.shader_program, "position_sampler");
	u_compute_scattering_texture_noise = glGetUniformLocation(shader_compute_scattering.shader_program, "noise_sampler");
	u_add_scattering_texture_color = glGetUniformLocation(shader_add_scattering.shader_program, "color_sampler");
	u_add_scattering_texture_scattering = glGetUniformLocation(shader_add_scattering.shader_program, "scattering_sampler");
	u_particle_texture_shadow = glGetUniformLocation(shader_particle.shader_program, "shadow_sampler");
	u_particle_texture_position = glGetUniformLocation(shader_particle.shader_program, "position_sampler");
	u_particle_texture_scattering = glGetUniformLocation(shader_particle.shader_program, "scattering_sampler");
	u_particle_time = glGetUniformLocation(shader_particle.shader_program, "time_uniform");

	//Surrounding cube
	const unsigned int surr_cube_num_vertices = 216;
	const float surr_cube_vertices[surr_cube_num_vertices] = {
		//Front
		-15.0f, +15.0f, +15.0f,	+0.0f, +0.0f, -1.0f,	0.2f, 0.2f, 0.4f,
		-15.0f, -3.0f, +15.0f,	+0.0f, +0.0f, -1.0f,	0.2f, 0.2f, 0.4f,
		+15.0f, -3.0f, +15.0f,	+0.0f, +0.0f, -1.0f,	0.2f, 0.2f, 0.4f,
		+15.0f, +15.0f, +15.0f,	+0.0f, +0.0f, -1.0f,	0.2f, 0.2f, 0.4f,
		//Back
		+15.0f, +15.0f, -15.0f,	+0.0f, +0.0f, +1.0f,	0.2f, 0.2f, 0.4f,
		+15.0f, -3.0f, -15.0f,	+0.0f, +0.0f, +1.0f,	0.2f, 0.2f, 0.4f,
		-15.0f, -3.0f, -15.0f,	+0.0f, +0.0f, +1.0f,	0.2f, 0.2f, 0.4f,
		-15.0f, +15.0f, -15.0f,	+0.0f, +0.0f, +1.0f,	0.2f, 0.2f, 0.4f,
		//Left
		-15.0f, +15.0f, -15.0f, +1.0f, +0.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		-15.0f, -3.0f, -15.0f,	+1.0f, +0.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		-15.0f, -3.0f, +15.0f,	+1.0f, +0.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		-15.0f, +15.0f, +15.0f,	+1.0f, +0.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		//Right
		+15.0f, +15.0f, +15.0f,	-1.0f, +0.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		+15.0f, -3.0f, +15.0f,	-1.0f, +0.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		+15.0f, -3.0f, -15.0f,	-1.0f, +0.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		+15.0f, +15.0f, -15.0f,	-1.0f, +0.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		//Top
		-15.0f, +15.0f, -15.0f,	+0.0f, -1.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		-15.0f, +15.0f, +15.0f,	+0.0f, -1.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		+15.0f, +15.0f, +15.0f,	+0.0f, -1.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		+15.0f, +15.0f, -15.0f,	+0.0f, -1.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		//Bottom
		-15.0f, -3.0f, +15.0f,	+0.0f, +1.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		-15.0f, -3.0f, -15.0f,	+0.0f, +1.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		+15.0f, -3.0f, -15.0f,	+0.0f, +1.0f, +0.0f,	0.2f, 0.2f, 0.4f,
		+15.0f, -3.0f, +15.0f,	+0.0f, +1.0f, +0.0f,	0.2f, 0.2f, 0.4f
	};
	const unsigned int surr_cube_num_indices = 36;
	const unsigned int surr_cube_indices[surr_cube_num_indices] = {
		2, 1, 0, 0, 3, 2,
		6, 5, 4, 4, 7, 6,
		10, 9, 8, 8, 11, 10,
		14, 13, 12, 12, 15, 14,
		18, 17, 16, 16, 19, 18,
		22, 21, 20, 20, 23, 22,
	};
	LoadGeometry(&surr_cube_vao, surr_cube_vertices, surr_cube_num_vertices, surr_cube_indices, surr_cube_num_indices, &surr_cube_ibo, VertexDataLayout::VERTEX_NORMAL_COLOR);
	//Blocking plane light
	const unsigned int blocking_plane_num_vertices = 72;
	const float blocking_plane_light_vertices[blocking_plane_num_vertices] = {
		//Large
		-15.0f, +15.0f, +15.0f,		+0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f, //Top left
		+15.0f, -3.0f,  +15.0f,		+0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f, //Bottom left
		+15.0f, -3.0f,  -15.0f,		+0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f, //Bottom right
		-15.0f, +15.0f, -15.0f,		+0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f, //Top right

		//Small
		Lerp(-15.0f, +15.0f, 0.5f), Lerp(-3.0f, +15.0f, 0.5f), Lerp(-15.0f, +15.0f, 0.55f),	 +0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f, //Top left
		Lerp(-15.0f, +15.0f, 0.6f), Lerp(-3.0f, +15.0f, 0.4f), Lerp(-15.0f, +15.0f, 0.55f),	 +0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f, //Bottom left
		Lerp(-15.0f, +15.0f, 0.6f), Lerp(-3.0f, +15.0f, 0.4f), Lerp(-15.0f, +15.0f, 0.45f),	 +0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f, //Bottom right
		Lerp(-15.0f, +15.0f, 0.5f), Lerp(-3.0f, +15.0f, 0.5f), Lerp(-15.0f, +15.0f, 0.45f),	 +0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f  //Top right
		/*+1.0f, +5.0f, +1.0f,		+0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f, //Top left
		+3.0f, +3.0f, +1.0f,		+0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f, //Bottom left
		+3.0f, +3.0f, -1.0f,		+0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f, //Bottom right
		+1.0f, +5.0f, -1.0f,		+0.5f, +0.5f, +0.0f,	+0.5f, +0.0f, +0.5f  //Top right*/
	};
	const unsigned int blocking_plane_num_indices = 24;
	const unsigned int blocking_plane_light_indices[blocking_plane_num_indices] = {
		0, 5, 4, 0, 1, 5, //Left side
		1, 6, 5, 1, 2, 6, //Bottom side
	  	2, 7, 6, 2, 3, 7, //Right side
		3, 4, 7, 3, 0, 4 //Top side
	};
	LoadGeometry(&blocking_plane_light_vao, blocking_plane_light_vertices, blocking_plane_num_vertices, blocking_plane_light_indices, blocking_plane_num_indices, &blocking_plane_light_ibo, VertexDataLayout::VERTEX_NORMAL_COLOR);
	//Blocking plane camera
	const unsigned int blocking_plane_camera_indices[blocking_plane_num_indices] = {
		0, 4, 5, 0, 5, 1, //Left side
		1, 5, 6, 1, 6, 2, //Bottom side
	  	2, 6, 7, 2, 7, 3, //Right side
		3, 7, 4, 3, 4, 0 //Top side
	};
	LoadGeometry(&blocking_plane_camera_vao, blocking_plane_light_vertices, blocking_plane_num_vertices, blocking_plane_camera_indices, blocking_plane_num_indices, &blocking_plane_camera_ibo, VertexDataLayout::VERTEX_NORMAL_COLOR);
	//Cube
	const unsigned int cube_num_vertices = 216;
	const float cube_vertices[cube_num_vertices] = {
		//Front
		-0.5f, +0.5f, +0.5f,	+0.0f, +0.0f, +1.0f,	1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, +0.5f,	+0.0f, +0.0f, +1.0f,	1.0f, 0.0f, 0.0f,
		+0.5f, -0.5f, +0.5f,	+0.0f, +0.0f, +1.0f,	1.0f, 0.0f, 0.0f,
		+0.5f, +0.5f, +0.5f,	+0.0f, +0.0f, +1.0f,	1.0f, 0.0f, 0.0f,
		//Back
		+0.5f, +0.5f, -0.5f,	+0.0f, +0.0f, -1.0f,	1.0f, 0.0f, 0.0f,
		+0.5f, -0.5f, -0.5f,	+0.0f, +0.0f, -1.0f,	1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	+0.0f, +0.0f, -1.0f,	1.0f, 0.0f, 0.0f,
		-0.5f, +0.5f, -0.5f,	+0.0f, +0.0f, -1.0f,	1.0f, 0.0f, 0.0f,
		//Left
		-0.5f, +0.5f, -0.5f,	-1.0f, +0.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f, +0.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, +0.5f,	-1.0f, +0.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		-0.5f, +0.5f, +0.5f,	-1.0f, +0.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		//Right
		+0.5f, +0.5f, +0.5f,	+1.0f, +0.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		+0.5f, -0.5f, +0.5f,	+1.0f, +0.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		+0.5f, -0.5f, -0.5f,	+1.0f, +0.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		+0.5f, +0.5f, -0.5f,	+1.0f, +0.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		//Top
		-0.5f, +0.5f, -0.5f,	+0.0f, +1.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		-0.5f, +0.5f, +0.5f,	+0.0f, +1.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		+0.5f, +0.5f, +0.5f,	+0.0f, +1.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		+0.5f, +0.5f, -0.5f,	+0.0f, +1.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		//Bottom
		-0.5f, -0.5f, +0.5f,	+0.0f, -1.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	+0.0f, -1.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		+0.5f, -0.5f, -0.5f,	+0.0f, -1.0f, +0.0f,	1.0f, 0.0f, 0.0f,
		+0.5f, -0.5f, +0.5f,	+0.0f, -1.0f, +0.0f,	1.0f, 0.0f, 0.0f
	};
	const unsigned int cube_num_indices = 36;
	const unsigned int cube_indices[cube_num_indices]{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20,
	};
	LoadGeometry(&cube_vao, cube_vertices, cube_num_vertices, cube_indices, cube_num_indices, &cube_ibo, VertexDataLayout::VERTEX_NORMAL_COLOR);
	//Statue
	LoadOBJ("assets/obj/Alucy.obj", &alucy, glm::vec3(0.5f, 0.2f, 0.2f));
	LoadGeometry(&alucy_vao, &alucy.vertex_buffer[0], alucy.num_vertices, &alucy.index_buffer[0], alucy.num_indices, &alucy_ibo, VertexDataLayout::VERTEX_NORMAL_COLOR);
	//Quad
	GLfloat quad_vertices[20] = {
		-1.0f, +1.0f, +0.0f,	+0.0f, +1.0f,
		-1.0f, -1.0f, +0.0f,	+0.0f, +0.0f,
		+1.0f, -1.0f, +0.0f,	+1.0f, +0.0f,
		+1.0f, +1.0f, +0.0f,	+1.0f, +1.0f
	};
	GLuint quad_indices[6] = { 0, 1, 2, 2, 3, 0 };
	LoadGeometry(&quad_vao, quad_vertices, 20, quad_indices, 6, &quad_ibo, VertexDataLayout::VERTEX_UV);
	//Particle
	const float particle_vertices[cube_num_vertices] = {
		//Front
		-0.005f, +0.005f, +0.005f,	+0.0f, +0.0f, +1.0f,
		-0.005f, -0.005f, +0.005f,	+0.0f, +0.0f, +1.0f,
		+0.005f, -0.005f, +0.005f,	+0.0f, +0.0f, +1.0f,
		+0.005f, +0.005f, +0.005f,	+0.0f, +0.0f, +1.0f,
		//Back
		+0.005f, +0.005f, -0.005f,	+0.0f, +0.0f, -1.0f,
		+0.005f, -0.005f, -0.005f,	+0.0f, +0.0f, -1.0f,
		-0.005f, -0.005f, -0.005f,	+0.0f, +0.0f, -1.0f,
		-0.005f, +0.005f, -0.005f,	+0.0f, +0.0f, -1.0f,
		//Left
		-0.005f, +0.005f, -0.005f,	-1.0f, +0.0f, +0.0f,
		-0.005f, -0.005f, -0.005f,	-1.0f, +0.0f, +0.0f,
		-0.005f, -0.005f, +0.005f,	-1.0f, +0.0f, +0.0f,
		-0.005f, +0.005f, +0.005f,	-1.0f, +0.0f, +0.0f,
		//Right
		+0.005f, +0.005f, +0.005f,	+1.0f, +0.0f, +0.0f,
		+0.005f, -0.005f, +0.005f,	+1.0f, +0.0f, +0.0f,
		+0.005f, -0.005f, -0.005f,	+1.0f, +0.0f, +0.0f,
		+0.005f, +0.005f, -0.005f,	+1.0f, +0.0f, +0.0f,
		//Top
		-0.005f, +0.005f, -0.005f,	+0.0f, +1.0f, +0.0f,
		-0.005f, +0.005f, +0.005f,	+0.0f, +1.0f, +0.0f,
		+0.005f, +0.005f, +0.005f,	+0.0f, +1.0f, +0.0f,
		+0.005f, +0.005f, -0.005f,	+0.0f, +1.0f, +0.0f,
		//Bottom
		-0.005f, -0.005f, +0.005f,	+0.0f, -1.0f, +0.0f,
		-0.005f, -0.005f, -0.005f,	+0.0f, -1.0f, +0.0f,
		+0.005f, -0.005f, -0.005f,	+0.0f, -1.0f, +0.0f,
		+0.005f, -0.005f, +0.005f,	+0.0f, -1.0f, +0.0f
};
	const unsigned int particle_indices[cube_num_indices]{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20,
	};
	LoadGeometry(&particle_vao, particle_vertices, 144, particle_indices, cube_num_indices, &particle_ibo, VertexDataLayout::VERTEX_NORMAL);
	Particle* particles = (Particle*)malloc(num_particles_x * num_particles_y * num_particles_z * sizeof(Particle)); //Size according to surrounding cube
	for (int z = -15; z < 15; z++)
	{
		for (int y = -3; y < 15; y++)
		{
			unsigned int z_offset = (z + 15) * num_particles_y * num_particles_x;
			unsigned int z_and_y_offset = z_offset + (y + 3) * num_particles_x;
			for (int x = -15; x < 15; x++)
			{
				unsigned int offset = z_and_y_offset + (x + 15);
				Particle p;
				float random_x = float(rand() % 31 - 15); //[-15,15]
				float random_y = float(rand() % 19 - 3);  //[-3,15]
				float random_z = float(rand() % 31 - 15); //[-15,15]
				p.position = glm::vec3(random_x, random_y, random_z);
				glm::vec3 direction = glm::vec3(float(rand() % 101) / 100.0f, float(rand() % 101) / 100.0f, float(rand() % 101) / 100.0f); //Rand [0,1]
				p.model_matrix = glm::translate(direction);
				particles[offset] = p;
			}
		}
	}
	glGenBuffers(1, &particle_centers_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, particle_centers_vbo);
	glBufferData(GL_ARRAY_BUFFER, num_particles_x * num_particles_y * num_particles_z * sizeof(Particle), particles, GL_STREAM_DRAW);
	glFinish();
	free(particles);
	glBindVertexArray(particle_vao);
	glEnableVertexAttribArray(2); //2 as it already has position=0 and normal=1
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 19, 0); //Particle has 9 floats
	glVertexAttribDivisor(2, 1); //Consume one each instance
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Shadow framebuffer and texture
	glGenTextures(1, &texture_shadow);
	glBindTexture(GL_TEXTURE_2D, texture_shadow);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glGenFramebuffers(1, &fbo_shadow);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_shadow);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture_shadow, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FRAMEBUFFER);

	//GBuffer
	glGenFramebuffers(1, &fbo_gbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_gbuffer);
	glGenTextures(1, &texture_depth);
	glBindTexture(GL_TEXTURE_2D, texture_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture_depth, 0);
	glGenTextures(1, &texture_color);
	glBindTexture(GL_TEXTURE_2D, texture_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color, 0);
	glGenTextures(1, &texture_position);
	glBindTexture(GL_TEXTURE_2D, texture_position);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture_position, 0);
	GLenum gbuffer_attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, gbuffer_attachments);
	glReadBuffer(GL_NONE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FRAMEBUFFER);

	//Scattering
	glGenFramebuffers(1, &fbo_scattering);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_scattering);
	glGenTextures(1, &texture_scattering);
	glBindTexture(GL_TEXTURE_2D, texture_scattering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, screen_width / 2, screen_height / 2, 0, GL_RED, GL_FLOAT, NULL); //Half the size of screen
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_scattering, 0);
	GLenum lightshaft_attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, lightshaft_attachments);
	glReadBuffer(GL_NONE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FRAMEBUFFER);
	lightshaft_basic = false; //If true ->lightshafts are slow

	//Noise textures
	LoadTexture("assets/textures/perlin_noise.png", 4, false, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, &texture_perlin_noise);
	LoadTexture("assets/textures/blue_noise.png", 4, false, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, &texture_blue_noise);
}

void Window::Update()
{
	auto frame_start = std::chrono::steady_clock::now();

	//Updates
	CheckForEvents();
	//View and projection matrices
	glm::mat4 camera_view_matrix = camera.WorldToViewMatrix();
	glm::mat4 camera_vp = camera_perspective_matrix * camera_view_matrix;
	lights[0].dir = glm::normalize(glm::vec3(0.0f) - lights[0].position); //Always look at center
	lights[0].vp = glm::perspective(70.0f, 1.0f, NEAR_PLANE, FAR_PLANE) * glm::lookAt(lights[0].position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //Always look at center
	//Model matrices
	glm::mat4 surr_cube_model_matrix = glm::mat4(1.0f);
	glm::mat4 blocking_plane_model_matrix_light = glm::mat4(1.0f);
	glm::mat4 blocking_plane_model_matrix_camera = glm::mat4(1.0f);
	glm::mat4 cube_model_matrix = glm::translate(glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 alucy_model_matrix = glm::translate(glm::vec3(-1.0f, -3.2f, 0.0f)) * glm::scale(glm::vec3(0.005f, 0.005f, 0.005f));
	glm::mat4 particle_model_matrix = glm::scale(glm::vec3(0.001f, 0.001f, 0.001f));
	//Update UBO
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	UBOData* ptr = (UBOData*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	if (ptr)
	{
		//General stuff
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		float viewportf[4] = { viewport[0], viewport[1], viewport[2], viewport[3] };
		memcpy(&ptr->viewport, &viewportf[0], 4 * sizeof(float));
		memcpy(&ptr->camera_vp, &camera_vp[0][0], 16 * sizeof(float));
		memcpy(&ptr->camera_to_world, &glm::inverse(camera_view_matrix)[0][0], 16 * sizeof(float));
		memcpy(&ptr->camera_pos, &glm::vec4(camera.position, 0.0f)[0], 4 * sizeof(float));
		memcpy(&ptr->camera_dir, &glm::vec4(camera.view_direction, 0.0f)[0], 4 * sizeof(float));
		//Light 0
		memcpy(&ptr->light_pos_0, &glm::vec4(lights[0].position, 0.0f)[0], 4 * sizeof(float));
		memcpy(&ptr->light_dir_0, &glm::vec4(lights[0].dir, 0.0f)[0], 4 * sizeof(float));
		memcpy(&ptr->light_cutoff_0, &glm::vec4(lights[0].cutoff, 0.0f, 0.0f, 0.0f)[0], 4 * sizeof(float));
		memcpy(&ptr->light_color_0, &glm::vec4(lights[0].color, 0.0f)[0], 4 * sizeof(float));
		memcpy(&ptr->light_vp_0, &lights[0].vp[0][0], 16 * sizeof(float));
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//Update particles
	glBindBuffer(GL_ARRAY_BUFFER, particle_centers_vbo);
	Particle* particle_ptr = (Particle*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (particle_ptr)
	{
		for (int z = -15; z < 15; z++)
		{
			for (int y = -3; y < 15; y++)
			{
				unsigned int z_offset = (z + 15) * num_particles_y * num_particles_x;
				unsigned int z_and_y_offset = z_offset + (y + 3) * num_particles_x;
				for (int x = -15; x < 15; x++)
				{
					unsigned int offset = z_and_y_offset + (x + 15);
					glm::vec3 position_offset = glm::vec3(particle_ptr[offset].model_matrix * glm::vec4(1.0f)) * (frame_time * 0.1f); //Decrease movement by a factor of 0.1
					particle_ptr[offset].position += position_offset;
					if (particle_ptr[offset].position.x < -15.0f || particle_ptr[offset].position.x > 15.0f ||
						particle_ptr[offset].position.y < -3.0f  || particle_ptr[offset].position.y > 15.0f ||
						particle_ptr[offset].position.z < -15.0f || particle_ptr[offset].position.z > 15.0f)
					{
						float random_x = float(rand() % 31 - 15); //[-15,15]
						float random_y = float(rand() % 19 - 3);  //[-3,15]
						float random_z = float(rand() % 31 - 15); //[-15,15]
						particle_ptr[offset].position = glm::vec3(random_x, random_y, random_z);
					}
				}
			}
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	////////////////////////////////////////
	///////////////RENDER///////////////////
	////////////////////////////////////////
	//Shadow pass
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_shadow);
	glClear(GL_DEPTH_BUFFER_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
	glUseProgram(shader_shadow.shader_program);
	glBindVertexArray(surr_cube_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surr_cube_ibo);
	glUniformMatrix4fv(u_shadow_model_matrix, 1, GL_FALSE, &surr_cube_model_matrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(blocking_plane_camera_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blocking_plane_camera_ibo);
	glUniformMatrix4fv(u_shadow_model_matrix, 1, GL_FALSE, &blocking_plane_model_matrix_camera[0][0]);
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(blocking_plane_light_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blocking_plane_light_ibo);
	glUniformMatrix4fv(u_shadow_model_matrix, 1, GL_FALSE, &blocking_plane_model_matrix_light[0][0]);
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(alucy_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, alucy_ibo);
	glUniformMatrix4fv(u_shadow_model_matrix, 1, GL_FALSE, &alucy_model_matrix[0][0]);
	glDrawElements(GL_TRIANGLES, alucy.num_indices, GL_UNSIGNED_INT, 0);
	/*glBindVertexArray(cube_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo);
	glUniformMatrix4fv(u_shadow_model_matrix, 1, GL_FALSE, &cube_model_matrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FRAMEBUFFER);
	glViewport(0, 0, screen_width, screen_height);

	//GBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_gbuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
	glUseProgram(shader_gbuffer.shader_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_shadow);
	glUniform1i(u_gbuffer_texture_shadow, 0);
	glBindVertexArray(surr_cube_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surr_cube_ibo);
	glUniformMatrix4fv(u_gbuffer_model_matrix, 1, GL_FALSE, &surr_cube_model_matrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(blocking_plane_camera_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blocking_plane_camera_ibo);
	glUniformMatrix4fv(u_shadow_model_matrix, 1, GL_FALSE, &blocking_plane_model_matrix_camera[0][0]);
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(blocking_plane_light_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blocking_plane_light_ibo);
	glUniformMatrix4fv(u_gbuffer_model_matrix, 1, GL_FALSE, &blocking_plane_model_matrix_light[0][0]);
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(alucy_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, alucy_ibo);
	glUniformMatrix4fv(u_gbuffer_model_matrix, 1, GL_FALSE, &alucy_model_matrix[0][0]);
	glDrawElements(GL_TRIANGLES, alucy.num_indices, GL_UNSIGNED_INT, 0);
	/*glBindVertexArray(cube_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo);
	glUniformMatrix4fv(u_gbuffer_model_matrix, 1, GL_FALSE, &cube_model_matrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FRAMEBUFFER);

	//Lightshaft pass
	if (lightshaft_basic)
	{
		auto ls_start = std::chrono::steady_clock::now();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
		glUseProgram(shader_lightshaft.shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_shadow);
		glUniform1i(u_lightshaft_texture_shadow, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_color);
		glUniform1i(u_lightshaft_texture_color, 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texture_position);
		glUniform1i(u_lightshaft_texture_position, 2);
		glBindVertexArray(quad_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ibo);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
		glFinish();
		long long ls_frame_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - ls_start).count(); //ms
		printf("%i\n", ls_frame_time_ms);
	}
	else
	{
		auto ls_start = std::chrono::steady_clock::now();
		glViewport(0, 0, screen_width / 2, screen_height / 2);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_scattering);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
		glUseProgram(shader_compute_scattering.shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_shadow);
		glUniform1i(u_compute_scattering_texture_shadow, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_position);
		glUniform1i(u_compute_scattering_texture_position, 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texture_perlin_noise);
		glUniform1i(u_compute_scattering_texture_noise, 2);
		glBindVertexArray(quad_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ibo);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FRAMEBUFFER);
		glViewport(0, 0, screen_width, screen_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
		glUseProgram(shader_add_scattering.shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_color);
		glUniform1i(u_add_scattering_texture_color, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_scattering);
		glUniform1i(u_add_scattering_texture_scattering, 1);
		glBindVertexArray(quad_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ibo);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);

		glFinish();
		long long ls_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - ls_start).count(); //ms
		printf("Lightshaft time: %i\n", ls_time_ms);
	}

	//Particle pass
	glClear(GL_DEPTH_BUFFER_BIT); //Only clear depth buffer as we're adding particles to the finished lightshafts
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
	glUseProgram(shader_particle.shader_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_shadow);
	glUniform1i(u_particle_texture_shadow, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_depth);
	glUniform1i(u_particle_texture_position, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture_scattering);
	glUniform1i(u_particle_texture_scattering, 2);
	glUniform1f(u_particle_time, total_time);
	glBindVertexArray(particle_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particle_ibo);
	glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, num_particles_x * num_particles_y * num_particles_z);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);

	glFinish();
	long long frame_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - frame_start).count(); //ms
	frame_time = float(frame_time_ms) / 1000.0f;
	total_time += frame_time;
	printf("Frame time: %i\n", frame_time_ms);

	SDL_GL_SwapWindow(window);
}

void Window::CheckForEvents()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT:
				is_closed = true;
				break;

			case SDL_KEYDOWN:
				switch (e.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						is_closed = true;
						break;
					case SDLK_SPACE:
						camera.rotate ^= 1; //XOR to switch rotate state
						break;
					case SDLK_l:
						lightshaft_basic ^= 1; //XOR which lightshaft method to use
						break;
					case SDLK_y:
						lights[0].position.y += 0.25f;
						break;
					case SDLK_h:
						lights[0].position.y -= 0.25f;
						break;
					case SDLK_j:
						lights[0].position.x += 0.25f;
						break;
					case SDLK_g:
						lights[0].position.x -= 0.25f;
						break;
					default: //Update camera position
						camera.UpdatePosition(e.key.keysym.sym);
						break;
				}
			case SDL_MOUSEMOTION:
				camera.UpdateViewDirection(e);
				break;
			default:
				break;
		}
	}
}