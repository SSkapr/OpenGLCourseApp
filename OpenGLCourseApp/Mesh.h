#pragma once

#include <GL\glew.h>

class Mesh
{
public:
	void CreateMesh(GLfloat *vertices, unsigned int *indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void RenderMesh();
	void ClearMesh();

	Mesh();
	~Mesh();

private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;
};

