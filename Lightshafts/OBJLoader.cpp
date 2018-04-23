#include <fstream>
#include "OBJLoader.h"
#include "StringUtilities.h"

//https://en.wikipedia.org/wiki/Wavefront_.obj_file
//http://paulbourke.net/dataformats/obj/

void CountMeshDetails(std::ifstream& reader, int* mesh_details);
void ExtractFaces(const std::string& line, int* faces, const unsigned short start_index, unsigned short num_numbers, bool has_uvs, bool has_normals);

int LoadOBJ(const std::string& file, Mesh* mesh, glm::vec3 color)
{
    std::ifstream reader(file);
    if (!reader.is_open())
    {
		printf("Failed to open OBJ file %s\n", file.c_str());
        return 1;
    }

	//Gather info about mesh (number of vertices, etc.)
	int mesh_details[4] = { 0, 0, 0, 0}; //[0]=num_vertices, [1]=num_uvs, [2]=num_normals, [3]=num_triangle_faces
	CountMeshDetails(reader, mesh_details);

	glm::vec3* vertices = (glm::vec3*)malloc(mesh_details[0] * sizeof(glm::vec3));
	glm::vec2* uvs = (glm::vec2*)malloc(mesh_details[1] * sizeof(glm::vec2));
	glm::vec3* normals = (glm::vec3*)malloc(mesh_details[2] * sizeof(glm::vec3));
	unsigned int* triangle_faces_vertices = (unsigned int*)malloc(mesh_details[3] * 3 * sizeof(unsigned int)); //*3 because 3 vertices per triangle face
	unsigned int* triangle_faces_uvs = (unsigned int*)malloc(mesh_details[3] * 3 * sizeof(unsigned int)); //*3 because 3 vertices per triangle face
	unsigned int* triangle_faces_normals = (unsigned int*)malloc(mesh_details[3] *3 * sizeof(unsigned int)); //*3 because 3 vertices per triangle face
	bool has_normals = false, has_uvs = false;
    
    std::string line;
	unsigned int vertex_index = 0, uv_index = 0, normal_index = 0;
	unsigned int triangle_faces_vertices_index = 0, triangle_faces_uvs_index = 0, triangle_faces_normals_index = 0;
    while (std::getline(reader, line))
    {
        if (line[0] == '#' || line.empty()) { continue; } //Comment
        else if (line[0] == 'v' && line[1] == ' ') //Vertex
        {
            unsigned short start_index = FindFirstNonSpace(line, 2); //2 becuase two characters are consumed by the initial condition that passed
            float points[3];
            ExtractPoints(line, &points[0], 3, start_index);
			vertices[vertex_index++] = glm::vec3(points[0], points[1], points[2]);
        }
        else if (line[0] == 'v' && line[1] == 't') //UV
        {
            has_uvs = true;
            unsigned short start_index = FindFirstNonSpace(line, 3); //3 becuase three characters are consumed by the initial condition that passed
            float points[2];
            ExtractPoints(line, &points[0], 2, start_index);
			uvs[uv_index++] = glm::vec2(points[0], points[1]);
        }
        else if (line[0] == 'v' && line[1] == 'n') //Normal
        {
            has_normals = true;
            unsigned short start_index = FindFirstNonSpace(line, 3); //3 becuase three characters are consumed by the initial condition that passed
            float points[3];
            ExtractPoints(line, &points[0], 3, start_index);
			normals[normal_index++] = glm::vec3(points[0], points[1], points[2]);
        }
        else if (line[0] == 'f') //Face indices
        {
            unsigned short start_index = FindFirstNonSpace(line, 2); //2 becuase two characters are consumed by the initial condition that passed ("f ")
			unsigned short num_spaces = CountSpaces(line, start_index);
			if (num_spaces == 2) //Triangle
			{
				if (has_uvs && has_normals) //f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
				{
					int tmp_faces[9]; //vertices&uvs&normals = 3*3
					ExtractFaces(line, &tmp_faces[0], start_index, 9, has_uvs, has_normals);
					for (int i = 0; i < 3; i++)
					{
						triangle_faces_vertices[triangle_faces_vertices_index++] = tmp_faces[i * 3 + 0];
						triangle_faces_uvs[triangle_faces_uvs_index++] = tmp_faces[i * 3 + 1];
						triangle_faces_normals[triangle_faces_normals_index++] = tmp_faces[i * 3 + 2];
					}
				}
				else if (has_uvs) //f v1 / vt1 v2 / vt2 v3 / vt3
				{
					int tmp_faces[6]; //vertices&uvs = 2*3
					ExtractFaces(line, &tmp_faces[0], start_index, 6, has_uvs, has_normals);
					for (int i = 0; i < 3; i++)
					{
						triangle_faces_vertices[triangle_faces_vertices_index++] = tmp_faces[i * 2 + 0];
						triangle_faces_uvs[triangle_faces_uvs_index++] = tmp_faces[i * 2 + 1];
					}
				}
				else if (has_normals) //f v1//vn1 v2//vn2 v3//vn3
				{
					int tmp_faces[6]; //vertices&normals = 2*3
					ExtractFaces(line, &tmp_faces[0], start_index, 6, has_uvs, has_normals);
					for (int i = 0; i < 3; i++)
					{
						triangle_faces_vertices[triangle_faces_vertices_index++] = tmp_faces[i * 2 + 0];
						triangle_faces_normals[triangle_faces_normals_index++] = tmp_faces[i * 2 + 1];
					}
				}
				else //f v1 v2 v3
				{
					int tmp_faces[3]; //vertices = 1*3
					ExtractFaces(line, &tmp_faces[0], start_index, 3, has_uvs, has_normals);
					for (int i = 0; i < 3; i++)
					{
						triangle_faces_vertices[triangle_faces_vertices_index++] = tmp_faces[i * 1 + 0];
					}
				}
			}
        }
    }
    reader.close();
    
	//Create data
	unsigned int num_points_per_vertex = 6;
	if (has_normals) { num_points_per_vertex += 3; }
	//Ignore uvs
	unsigned int i = 0;
	for (int it = 0; it < mesh_details[3]; it++) //For each triangle
	{
		//Vertices for triangle
		glm::vec3 v0 = vertices[triangle_faces_vertices[i]];
		glm::vec3 v1 = vertices[triangle_faces_vertices[i + 1]];
		glm::vec3 v2 = vertices[triangle_faces_vertices[i + 2]];
		//Normals for triangle
		if (has_normals)
		{
			glm::vec3 n0 = normals[triangle_faces_normals[i]];
			glm::vec3 n1 = normals[triangle_faces_normals[i + 1]];
			glm::vec3 n2 = normals[triangle_faces_normals[i + 2]];

			mesh->vertex_buffer.emplace_back(v0.x);
			mesh->vertex_buffer.emplace_back(v0.y);
			mesh->vertex_buffer.emplace_back(v0.z);
			mesh->vertex_buffer.emplace_back(n0.x);
			mesh->vertex_buffer.emplace_back(n0.y);
			mesh->vertex_buffer.emplace_back(n0.z);
			mesh->vertex_buffer.emplace_back(color.x);
			mesh->vertex_buffer.emplace_back(color.y);
			mesh->vertex_buffer.emplace_back(color.z);

			mesh->vertex_buffer.emplace_back(v1.x);
			mesh->vertex_buffer.emplace_back(v1.y);
			mesh->vertex_buffer.emplace_back(v1.z);
			mesh->vertex_buffer.emplace_back(n1.x);
			mesh->vertex_buffer.emplace_back(n1.y);
			mesh->vertex_buffer.emplace_back(n1.z);
			mesh->vertex_buffer.emplace_back(color.x);
			mesh->vertex_buffer.emplace_back(color.y);
			mesh->vertex_buffer.emplace_back(color.z);

			mesh->vertex_buffer.emplace_back(v2.x);
			mesh->vertex_buffer.emplace_back(v2.y);
			mesh->vertex_buffer.emplace_back(v2.z);
			mesh->vertex_buffer.emplace_back(n2.x);
			mesh->vertex_buffer.emplace_back(n2.y);
			mesh->vertex_buffer.emplace_back(n2.z);
			mesh->vertex_buffer.emplace_back(color.x);
			mesh->vertex_buffer.emplace_back(color.y);
			mesh->vertex_buffer.emplace_back(color.z);
		}
		else
		{
			mesh->vertex_buffer.emplace_back(v0.x);
			mesh->vertex_buffer.emplace_back(v0.y);
			mesh->vertex_buffer.emplace_back(v0.z);
			mesh->vertex_buffer.emplace_back(color.x);
			mesh->vertex_buffer.emplace_back(color.y);
			mesh->vertex_buffer.emplace_back(color.z);

			mesh->vertex_buffer.emplace_back(v1.x);
			mesh->vertex_buffer.emplace_back(v1.y);
			mesh->vertex_buffer.emplace_back(v1.z);
			mesh->vertex_buffer.emplace_back(color.x);
			mesh->vertex_buffer.emplace_back(color.y);
			mesh->vertex_buffer.emplace_back(color.z);

			mesh->vertex_buffer.emplace_back(v2.x);
			mesh->vertex_buffer.emplace_back(v2.y);
			mesh->vertex_buffer.emplace_back(v2.z);
			mesh->vertex_buffer.emplace_back(color.x);
			mesh->vertex_buffer.emplace_back(color.y);
			mesh->vertex_buffer.emplace_back(color.z);
		}

		mesh->index_buffer.emplace_back(i++);
		mesh->index_buffer.emplace_back(i++);
		mesh->index_buffer.emplace_back(i++);
	}
	mesh->num_vertices = (int)mesh->vertex_buffer.size();
	mesh->num_indices = i;

	//Bools
    mesh->has_uvs = has_uvs;
    mesh->has_normals = has_normals;

	//Clean up
	free(vertices);
	free(uvs);
	free(normals);
	free(triangle_faces_vertices);
	free(triangle_faces_uvs);
	free(triangle_faces_normals);
    
    return 0;
}

void CountMeshDetails(std::ifstream& reader, int* mesh_details)
{
	std::string line;
	while (std::getline(reader, line))
	{
		if (line[0] == '#' || line.empty()) { continue; } //Comment or empty line
		else if (line[0] == 'v' && line[1] == ' ') //Vertex
		{
			mesh_details[0]++;
		}
		else if (line[0] == 'v' && line[1] == 't') //UV
		{
			mesh_details[1]++;
		}
		else if (line[0] == 'v' && line[1] == 'n') //Normal
		{
			mesh_details[2]++;
		}
		else if (line[0] == 'f') //Face
		{
			const int num_spaces = CountSpaces(line, 2);
			if (num_spaces == 2) //Triangle face
			{
				mesh_details[3]++;
			}
		}
	}

	//Reset reader
	reader.clear();
	reader.seekg(0, std::ios::beg);
}

void ExtractFaces(const std::string& line, int* faces, const unsigned short start_index, unsigned short num_numbers, bool has_uvs, bool has_normals)
{
    unsigned short numbers_found = 0;
    unsigned short start = start_index;
    unsigned short i = start_index;
    if (has_uvs && has_normals) //f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
    {
        while (i < line.size() && numbers_found < num_numbers - 3) //Find all but last three
        {
            if (line[i] == ' ')
            {
                unsigned short mid_1 = start;
                while (line[mid_1] != '/')
                {
                    mid_1++;
                }
                unsigned short mid_2 = mid_1 + 1;
                while (line[mid_2] != '/')
                {
                    mid_2++;
                }
                faces[numbers_found++] = abs(atoi(line.substr(start, mid_1).c_str())) - 1;
                faces[numbers_found++] = abs(atoi(line.substr(mid_1 + 1, mid_2).c_str())) - 1;
                faces[numbers_found++] = abs(atoi(line.substr(mid_2 + 1, i).c_str())) - 1;
                start = i + 1;
            }
            i++;
        }
        unsigned short mid_1 = start;
        while (line[mid_1] != '/')
        {
            mid_1++;
        }
        unsigned short mid_2 = mid_1 + 1;
        while (line[mid_2] != '/')
        {
            mid_2++;
        }
        faces[numbers_found++] = abs(atoi(line.substr(start, mid_1).c_str())) - 1;
        faces[numbers_found++] = abs(atoi(line.substr(mid_1 + 1, mid_2).c_str())) - 1;
        faces[numbers_found++] = abs(atoi(line.substr(mid_2 + 1, i).c_str())) - 1;
    }
    else if (has_uvs) //f v1/vt1 v2/vt2 v3/vt3
    {
        while (i < line.size() && numbers_found < num_numbers - 2) //Find all but last two
        {
            if (line[i] == ' ')
            {
                unsigned short mid = start;
                while (line[mid] != '/')
                {
                    mid++;
                }
                faces[numbers_found++] = abs(atoi(line.substr(start, mid).c_str())) - 1;
                faces[numbers_found++] = abs(atoi(line.substr(mid + 1, i).c_str())) - 1;
                start = i + 1;
            }
            i++;
        }
        unsigned short mid = start;
        while (line[mid] != '/')
        {
            mid++;
        }
        faces[numbers_found++] = abs(atoi(line.substr(start, mid).c_str())) - 1;
        faces[numbers_found++] = abs(atoi(line.substr(mid + 1, i).c_str())) - 1;
    }
    else if (has_normals) //f v1//vn1 v2//vn2 v3//vn3
    {
        while (i < line.size() && numbers_found < num_numbers - 2) //Find all but last two
        {
            if (line[i] == ' ')
            {
                unsigned short mid = start;
                while (!(line[mid] == '/' && line[mid + 1] == '/'))
                {
                    mid++;
                }
                faces[numbers_found++] = abs(atoi(line.substr(start, mid + 1).c_str())) - 1;
                faces[numbers_found++] = abs(atoi(line.substr(mid + 2, i).c_str())) - 1;
                start = i + 1;
            }
            i++;
        }
        unsigned short mid = start;
        while (!(line[mid] == '/' && line[mid + 1] == '/'))
        {
            mid++;
        }
        faces[numbers_found++] = abs(atoi(line.substr(start, mid + 1).c_str())) - 1;
        faces[numbers_found++] = abs(atoi(line.substr(mid + 2, i).c_str())) - 1;
    }
    else //f v1 v2 v3
    {
        while (i < line.size() && numbers_found < num_numbers - 1) //Find all but last one
        {
            if (line[i] == ' ')
            {
                faces[numbers_found++] = abs(atoi(line.substr(start, i).c_str())) - 1;
                start = i + 1;
            }
            i++;
        }
        faces[numbers_found++] = abs(atoi(line.substr(start, line.size()).c_str())) - 1;
    }
}