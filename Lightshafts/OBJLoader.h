#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

typedef struct float2 {
	float x;
	float y;

	float2(float _x, float _y) : x(0), y(0)
	{
		x = _x;
		y = _y;
	}

	float2()
	{
		x = 0;
		y = 0;
	}
} float2;

typedef struct float3 {
	float x;
	float y;
	float z;

	float3(float _x, float _y, float _z) : x(0), y(0), z(0)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	float3()
	{
		x = 0;
		y = 0;
		z = 0;
	}
} float3;

typedef struct Mesh {
	float3* vertices;
	float3* normals;
	float2* textureCoordinates;

	unsigned int* indices;

	size_t vertexCount;
	size_t indexCount;
} Mesh;

Mesh loadOBJ(std::string src);