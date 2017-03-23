#ifndef GLPERMESH_H
#define GLPERMESH_H

#include <glad/glad.h>
#include <vector>
#include <Engine/Mesh.h>

class GLPerMesh {
public:
	GLPerMesh(const Engine::Mesh* mesh, GLint vertexPosition, GLint vertexNormal,
		GLint vertexTextureCoordinate);
	~GLPerMesh();

	void bind();
	void draw();

private:
	GLenum primitiveType;
	std::vector<GLuint> buffers;
	GLuint vao;
	uint32_t elementCount;
	bool indexed;
};

#endif
