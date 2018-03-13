#include <fstream>
#include <iostream>
#include "Shader.h"

GLuint CreateShader(const std::string& text, GLenum type, const std::string& filename);
std::string LoadShader(const std::string& filename);

void Shader::Init(const std::string& vertexShader, const std::string& fragmentShader)
{
	//Create shader program and its shaders
	shader_program = glCreateProgram();
	GLuint vertex_shader = CreateShader(LoadShader(vertexShader), GL_VERTEX_SHADER, vertexShader);
	GLuint fragment_shader = CreateShader(LoadShader(fragmentShader), GL_FRAGMENT_SHADER, fragmentShader);

	//Attach shaders to shader program
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);

	//Link shader program
	glLinkProgram(shader_program);

	//Check for linking errors
	GLint linkResult;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &linkResult);
	if (linkResult != GL_TRUE)
	{
		GLint infoLogLength;
		glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLint bufferSize; //Size of buffer that glGetProgramLog returns
		glGetProgramInfoLog(shader_program, infoLogLength, &bufferSize, buffer);
		std::cout << buffer << std::endl;
	}

	//Validate program
	glValidateProgram(shader_program);

	//Clean up shaders
	glDetachShader(shader_program, vertex_shader);
	glDetachShader(shader_program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

void Shader::Bind()
{
	glUseProgram(shader_program);
}

void Shader::Destroy()
{
	glUseProgram(0);
	glDeleteProgram(shader_program);
}

GLuint CreateShader(const std::string& text, GLenum type, const std::string& filename)
{
	GLuint shader = glCreateShader(type);

	//Char* to hold text from shader file - i.e vertexShader and fragmentShader
	const char* shaderText[1];
	shaderText[0] = text.c_str();

	//Define shader source
	glShaderSource(shader, 1, shaderText, 0);

	//Compile shader
	glCompileShader(shader);

	//Check for compile errors
	GLint compileResult;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
	if (compileResult != GL_TRUE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize; //Size of buffer that glGetShaderLog returns
		glGetShaderInfoLog(shader, infoLogLength, &bufferSize, buffer);
		std::cout << "In shader " << filename << ":  " << buffer << std::endl;

		delete buffer;
	}

	return shader;
}

std::string LoadShader(const std::string& filename)
{
	std::ifstream file;
	file.open((filename).c_str());

	std::string output;
	std::string line;

	if (file.is_open())
	{
		while (file.good())
		{
			getline(file, line);
			output.append(line + "\n");
		}
	}
	else
	{
		std::cerr << "Unable to load shader: " << filename << std::endl;
	}

	return output;
}