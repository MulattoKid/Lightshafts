#pragma once

#include "GLUtilities.h"
#include "include/glm/gtc/matrix_transform.hpp"
#include <time.h>
#include "Window.h"

#define NEAR_PLANE 0.1f
#define FAR_PLANE 1000.0f

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
	//Setup render state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//Shaders
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOData), NULL, GL_DYNAMIC_DRAW);
	shader_plain_normal.Init("assets/shaders/plain_normal.vert", "assets/shaders/plain_normal.frag");

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
	u_shader_plain_normal_ubo = glGetUniformBlockIndex(shader_plain_normal.shader_program, "UBOData");
	glUniformBlockBinding(shader_plain_normal.shader_program, u_shader_plain_normal_ubo, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);

	//Meshes
	const unsigned int plane_num_vertices = 24;
	const float plane_vertices[plane_num_vertices] = {
		-100.0f, -10.0f, -100.0f,	+0.0f, +1.0f, +0.0f,
		-100.0f, -10.0f, +100.0f,	+0.0f, +1.0f, +0.0f,
		+100.0f, -10.0f, +100.0f,	+0.0f, +1.0f, +0.0f,
		+100.0f, -10.0f, -100.0f,	+0.0f, +1.0f, +0.0f
	};
	const unsigned int plane_num_indices = 6;
	const unsigned int plane_indices[plane_num_indices] = { 0, 1, 2, 2, 3, 0 };
	LoadGeometry(&plane_vao, plane_vertices, plane_num_vertices, plane_indices, plane_num_indices, &plane_ibo, VertexDataLayout::VERTEX_NORMAL);

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

	//General stuff
	camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.25f);
	perspective_matrix = glm::perspective(glm::radians(70.0f), (float)screen_width / screen_height, NEAR_PLANE, FAR_PLANE);
	total_time = 0.0f, frame_time = 0.0f;
}

void Window::Update()
{
	auto start = std::chrono::steady_clock::now();
	CheckForEvents();

	//Updates
	glm::mat4 view_matrix = camera.WorldToViewMatrix();
	glm::mat4 vp = perspective_matrix * view_matrix;
	//Update UBO
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	UBOData* ptr = (UBOData*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	if (ptr)
	{
		memcpy(&ptr->vp, &vp[0][0], 16 * sizeof(float));
		//memcpy(&ptr->light_vp, ubo_data.light_vp, 16 * sizeof(float));
		//memcpy(&ptr->cam_position, ubo_data.cam_position, 4 * sizeof(float));
		//memcpy(&ptr->light_position, ubo_data.light_position, 4 * sizeof(float));
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//Clear default framebuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Render
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
	glUseProgram(shader_plain_normal.shader_program);
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
	
	SDL_GL_SwapWindow(window);

	auto end = std::chrono::steady_clock::now();
	long long frame_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); //ms
	frame_time = (float)frame_time_ms / 1000.0f; //s
	total_time += frame_time;
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