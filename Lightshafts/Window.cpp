#pragma once

#include "GLUtilities.h"
#include "include/glm/gtc/matrix_transform.hpp"
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

void Window::InitLights()
{
	//Light 0
	lights[0].position = glm::vec3(-2.0f, 2.0f, 0.0f);
	lights[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
	lights[0].vp = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, NEAR_PLANE, FAR_PLANE) * glm::lookAt(lights[0].position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Window::Init()
{
	//Camera
	camera = Camera(glm::vec3(0.0f, 2.0f, 10.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.25f, true);
	camera_perspective_matrix = glm::perspective(glm::radians(70.0f), (float)screen_width / screen_height, NEAR_PLANE, FAR_PLANE);

	//Light
	InitLights();
	light_perspective_matrix = glm::ortho(-10.0f, 10.0f, -1.0f, 1.0f, NEAR_PLANE, FAR_PLANE);

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
	shader_gbuffer_quad.Init("assets/shaders/gbuffer_quad.vert", "assets/shaders/gbuffer_quad.frag");
	shader_lightshaft.Init("assets/shaders/lightshaft.vert", "assets/shaders/lightshaft.frag");
	shader_quad.Init("assets/shaders/quad.vert", "assets/shaders/quad.frag");

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
	u_shader_quad_ubo = glGetUniformBlockIndex(shader_quad.shader_program, "UBOData");
	glUniformBlockBinding(shader_quad.shader_program, u_shader_quad_ubo, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);

	//Texture uniforms
	u_gbuffer_texture_shadow = glGetUniformLocation(shader_gbuffer.shader_program, "shadow_sampler");
	u_lightshaft_texture_shadow = glGetUniformLocation(shader_lightshaft.shader_program, "shadow_sampler");
	u_lightshaft_texture_color = glGetUniformLocation(shader_lightshaft.shader_program, "color_sampler");
	u_lightshaft_texture_position = glGetUniformLocation(shader_lightshaft.shader_program, "position_sampler");

	//Plane
	const unsigned int plane_num_vertices = 24;
	const float plane_vertices[plane_num_vertices] = {
		-100.0f, -4.0f, -100.0f,	+0.0f, +1.0f, +0.0f,
		-100.0f, -4.0f, +100.0f,	+0.0f, +1.0f, +0.0f,
		+100.0f, -4.0f, +100.0f,	+0.0f, +1.0f, +0.0f,
		+100.0f, -4.0f, -100.0f,	+0.0f, +1.0f, +0.0f
	};
	const unsigned int plane_num_indices = 6;
	const unsigned int plane_indices[plane_num_indices] = { 0, 1, 2, 2, 3, 0 };
	LoadGeometry(&plane_vao, plane_vertices, plane_num_vertices, plane_indices, plane_num_indices, &plane_ibo, VertexDataLayout::VERTEX_NORMAL);
	//Cube
	const unsigned int cube_num_vertices = 144;
	const float cube_vertices[cube_num_vertices] = {
		//Front
		-0.5f, +0.5f, +0.5f,	+0.0f, +0.0f, +1.0f,
		-0.5f, -0.5f, +0.5f,	+0.0f, +0.0f, +1.0f,
		+0.5f, -0.5f, +0.5f,	+0.0f, +0.0f, +1.0f,
		+0.5f, +0.5f, +0.5f,	+0.0f, +0.0f, +1.0f,
		//Back
		+0.5f, +0.5f, -0.5f,	+0.0f, +0.0f, -1.0f,
		+0.5f, -0.5f, -0.5f,	+0.0f, +0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,	+0.0f, +0.0f, -1.0f,
		-0.5f, +0.5f, -0.5f,	+0.0f, +0.0f, -1.0f,
		//Left
		-0.5f, +0.5f, -0.5f,	-1.0f, +0.0f, +0.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f, +0.0f, +0.0f,
		-0.5f, -0.5f, +0.5f,	-1.0f, +0.0f, +0.0f,
		-0.5f, +0.5f, +0.5f,	-1.0f, +0.0f, +0.0f,
		//Right
		+0.5f, +0.5f, +0.5f,	+1.0f, +0.0f, +0.0f,
		+0.5f, -0.5f, +0.5f,	+1.0f, +0.0f, +0.0f,
		+0.5f, -0.5f, -0.5f,	+1.0f, +0.0f, +0.0f,
		+0.5f, +0.5f, -0.5f,	+1.0f, +0.0f, +0.0f,
		//Top
		-0.5f, +0.5f, -0.5f,	+0.0f, +1.0f, +0.0f,
		-0.5f, +0.5f, +0.5f,	+0.0f, +1.0f, +0.0f,
		+0.5f, +0.5f, +0.5f,	+0.0f, +1.0f, +0.0f,
		+0.5f, +0.5f, -0.5f,	+0.0f, +1.0f, +0.0f,
		//Bottom
		-0.5f, -0.5f, +0.5f,	+0.0f, -1.0f, +0.0f,
		-0.5f, -0.5f, -0.5f,	+0.0f, -1.0f, +0.0f,
		+0.5f, -0.5f, -0.5f,	+0.0f, -1.0f, +0.0f,
		+0.5f, -0.5f, +0.5f,	+0.0f, -1.0f, +0.0f,
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
	LoadGeometry(&cube_vao, cube_vertices, cube_num_vertices, cube_indices, cube_num_indices, &cube_ibo, VertexDataLayout::VERTEX_NORMAL);

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
	/*glGenTextures(1, &texture_depth);
	glBindTexture(GL_TEXTURE_2D, texture_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture_depth, 0);*/
	glGenFramebuffers(1, &fbo_gbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_gbuffer);
	glGenTextures(1, &texture_color);
	glBindTexture(GL_TEXTURE_2D, texture_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color, 0);
	glGenTextures(1, &texture_position);
	glBindTexture(GL_TEXTURE_2D, texture_position);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL); //RGB32F/16F doesn't seem to work for whatever reason...
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture_position, 0);
	GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	glReadBuffer(GL_NONE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FRAMEBUFFER);

	//Quad
	GLfloat quad_vertices[20] = {
		-1.0f, +1.0f, +0.0f,	+0.0f, +1.0f,
		-1.0f, -1.0f, +0.0f,	+0.0f, +0.0f,
		+1.0f, -1.0f, +0.0f,	+1.0f, +0.0f,
		+1.0f, +1.0f, +0.0f,	+1.0f, +1.0f
	};
	GLuint quad_indices[6] = { 0, 1, 2, 2, 3, 0 };
	LoadGeometry(&quad_vao, quad_vertices, 20, quad_indices, 6, &quad_ibo, VertexDataLayout::VERTEX_UV);
}

void Window::Update()
{
	auto start = std::chrono::steady_clock::now();
	CheckForEvents();

	//Updates
	glm::mat4 camera_view_matrix = camera.WorldToViewMatrix();
	glm::mat4 camera_vp = camera_perspective_matrix * camera_view_matrix;
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
		memcpy(&ptr->light_color_0, &glm::vec4(lights[0].color, 0.0f)[0], 4 * sizeof(float));
		memcpy(&ptr->light_vp_0, &lights[0].vp[0][0], 16 * sizeof(float));
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	////////////////////////////////////////
	///////////////RENDER///////////////////
	////////////////////////////////////////
	//Shadow pass
	glCullFace(GL_FRONT);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_shadow);
	glClear(GL_DEPTH_BUFFER_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
	glUseProgram(shader_shadow.shader_program);
	glBindVertexArray(plane_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane_ibo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(cube_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FRAMEBUFFER);
	glViewport(0, 0, screen_width, screen_height);
	glCullFace(GL_BACK);

	//GBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_gbuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Render a quad with a "maximum" depth so that when sampling the gbuffer texture
	//we always get a point as the entire screen is always covered by a polygon
	glUseProgram(shader_gbuffer_quad.shader_program);
	glBindVertexArray(quad_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ibo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
	glUseProgram(shader_gbuffer.shader_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_shadow);
	glUniform1i(u_gbuffer_texture_shadow, 0);
	glBindVertexArray(plane_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane_ibo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(cube_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FRAMEBUFFER);

	//Lightshaft pass
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

	//Test quad
	/*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
	glUseProgram(shader_quad.shader_program);
	glBindVertexArray(quad_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ibo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);*/

	//Swap window
	SDL_GL_SwapWindow(window);

	//Frame end
	auto end = std::chrono::steady_clock::now();
	long long frame_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); //ms
	frame_time = (float)frame_time_ms / 1000.0f; //s
	total_time += frame_time;
	//printf("%i\n", frame_time_ms);
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