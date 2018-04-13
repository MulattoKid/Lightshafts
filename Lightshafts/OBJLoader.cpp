#include "OBJLoader.h"
#include <algorithm>

void split(std::vector<std::string>* parts, const std::string &s, char delim) {
	
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		parts->push_back(item);
	}
}

void deleteEmptyStrings(std::vector<std::string> &list) {
	for (int i = 0; i < list.size(); i++) {
		std::string item = list.at(i);
		if (item == "") {
			list.erase(list.begin() + i);
			i--;
		}
	}
}

Mesh loadOBJ(std::string src)
{
	std::vector<std::string> lineParts;
	lineParts.reserve(32);
	std::vector<std::string> faceParts;
	faceParts.reserve(32);
	
	std::vector<float3> vertices;
	std::vector<float3> normals;
	std::vector<float2> textureCoordinates;

	std::vector<float3> vertexBuffer;
	std::vector<float2> textureBuffer;
	std::vector<float3> normalBuffer;

	std::vector<unsigned int> indices;

	std::ifstream objFile(src);
	std::string line;

	unsigned int currentIndex = 0;

	if (objFile.is_open()) {


		int lineNumber = -1;
		while (std::getline(objFile, line)) {
			lineParts.clear();
			split(&lineParts, line, ' ');
			deleteEmptyStrings(lineParts);

			lineNumber++;


			if (lineParts.size() == 0) {
				continue;
			}

			if (lineParts.at(0) == "v") {
				float3 vertex;
				vertex.x = std::stof(lineParts.at(1));
				vertex.y = std::stof(lineParts.at(2));
				vertex.z = std::stof(lineParts.at(3));
				vertexBuffer.push_back(vertex);
			}

			if (lineParts.at(0) == "vn") {
				float3 normal;
				normal.x = std::stof(lineParts.at(1));
				normal.y = std::stof(lineParts.at(2));
				normal.z = std::stof(lineParts.at(3));
				normalBuffer.push_back(normal);
			}

			if (lineParts.at(0) == "vt") {
				float2 textureCoordinate;
				textureCoordinate.x = std::stof(lineParts.at(1));
				textureCoordinate.y = std::stof(lineParts.at(2));
				textureBuffer.push_back(textureCoordinate);
			}

			if (lineParts.at(0) == "f") {
				for (int i = 1; i <= 3; i++) {
					faceParts.clear();
					split(&faceParts, lineParts.at(i), '/');

					vertices.push_back(vertexBuffer.at(std::stoi(faceParts.at(0)) - 1));
					textureCoordinates.push_back(textureBuffer.at(std::stoi(faceParts.at(1)) - 1));
					normals.push_back(normalBuffer.at(std::stoi(faceParts.at(2)) - 1));

					indices.push_back(currentIndex);
					currentIndex++;
				}
			}
		}

		unsigned int faceCount = unsigned(indices.size()) / 3;

		float3* meshVertexBuffer = new float3[vertices.size()];
		std::copy(vertices.begin(), vertices.end(), meshVertexBuffer);

		float3* meshNormalBuffer = new float3[normals.size()];
		std::copy(normals.begin(), normals.end(), meshNormalBuffer);

		float2* meshTextureCoordBuffer = new float2[textureCoordinates.size()];
		std::copy(textureCoordinates.begin(), textureCoordinates.end(), meshTextureCoordBuffer);

		unsigned int* meshIndexBuffer = new unsigned int[3 * faceCount];
		std::copy(indices.begin(), indices.end(), meshIndexBuffer);

		objFile.close();

		Mesh mesh;

		mesh.vertices = meshVertexBuffer;
		mesh.normals = meshNormalBuffer;
		mesh.textureCoordinates = meshTextureCoordBuffer;

		mesh.indices = meshIndexBuffer;

		mesh.vertexCount = 3 * faceCount;
		mesh.indexCount = 3 * faceCount;

		return mesh;
	}
	else {
		std::cout << "Something went wrong reading the file! (probably the specified path was incorrect)" << std::endl;
	}

	return Mesh();
}
