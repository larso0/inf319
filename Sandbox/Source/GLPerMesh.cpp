#include "GLPerMesh.h"
#include <Engine/IndexedMesh.h>
#include <stdexcept>

using namespace std;
using namespace Engine;

static GLuint createVertexBuffer(const Mesh& mesh) {
	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(
	GL_ARRAY_BUFFER, mesh.getVertexDataSize(), mesh.getVertexData(),
	GL_STATIC_DRAW);
	return buf;
}

static GLuint createIndexBuffer(const IndexedMesh& mesh) {
	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(
	GL_ARRAY_BUFFER, mesh.getIndexDataSize(), mesh.getIndexData(),
	GL_STATIC_DRAW);
	return buf;
}

static GLuint createVAO(GLuint vertexBuffer, GLint vertexPosition,
	GLint vertexNormal, GLint vertexTextureCoordinate) {
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	if (vertexPosition >= 0) {
		glEnableVertexAttribArray(vertexPosition);
		glVertexAttribPointer(vertexPosition, 3, GL_FLOAT, GL_FALSE,
			(GLsizei)Vertex::Stride, (const GLvoid*) Vertex::PositionOffset);
	}

	if (vertexNormal >= 0) {
		glEnableVertexAttribArray(vertexNormal);
		glVertexAttribPointer(vertexNormal, 3, GL_FLOAT, GL_FALSE,
			(GLsizei)Vertex::Stride, (const GLvoid*) Vertex::NormalOffset);
	}

	if (vertexTextureCoordinate >= 0) {
		glEnableVertexAttribArray(vertexTextureCoordinate);
		glVertexAttribPointer(vertexTextureCoordinate, 2, GL_FLOAT, GL_FALSE,
			(GLsizei)Vertex::Stride, (const GLvoid*) Vertex::TextureCoordinateOffset);
	}

	return vao;
}

static GLuint createVAOIndexed(GLuint vertexBuffer, GLuint indexBuffer,
	GLint vertexPosition, GLint vertexNormal, GLint vertexTextureCoordinate) {
	GLuint vao = createVAO(vertexBuffer, vertexPosition, vertexNormal,
		vertexTextureCoordinate);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	return vao;
}

GLPerMesh::GLPerMesh(const Mesh* mesh, GLint vertexPosition, GLint vertexNormal,
	GLint vertexTextureCoordinate) :
indexed(false)
{
	switch (mesh->getTopology()) {
	case Mesh::Topology::Points: primitiveType = GL_POINTS; break;
	case Mesh::Topology::Lines: primitiveType = GL_LINES; break;
	case Mesh::Topology::LineStrip: primitiveType = GL_LINE_STRIP; break;
	case Mesh::Topology::Triangles: primitiveType = GL_TRIANGLES; break;
	case Mesh::Topology::TriangleStrip: primitiveType = GL_TRIANGLE_STRIP; break;
	case Mesh::Topology:: TriangleFan: primitiveType = GL_TRIANGLE_FAN; break;
	default:
		throw runtime_error("Unknown topology.");
	}

	elementCount = (uint32_t)mesh->getElementCount();
	GLuint vertexBuffer = createVertexBuffer(*mesh);
	buffers.push_back(vertexBuffer);
	const IndexedMesh* indexedMesh = dynamic_cast<const IndexedMesh*>(mesh);
	if (indexedMesh) {
		GLuint indexBuffer = createIndexBuffer(*indexedMesh);
		buffers.push_back(indexBuffer);
		vao = createVAOIndexed(vertexBuffer, indexBuffer,
			vertexPosition, vertexNormal, vertexTextureCoordinate);
		indexed = true;
	} else {
		vao = createVAO(vertexBuffer, vertexPosition,
			vertexNormal, vertexTextureCoordinate);
	}
}

GLPerMesh::~GLPerMesh() {
	glDeleteBuffers((GLsizei)buffers.size(), buffers.data());
	glDeleteVertexArrays(1, &vao);
}

void GLPerMesh::bind() {
	glBindVertexArray(vao);
}

void GLPerMesh::draw() {
	if (indexed) {
		glDrawElements(primitiveType, elementCount, GL_UNSIGNED_INT, 0);
	} else {
		glDrawArrays(primitiveType, 0, elementCount);
	}
}

