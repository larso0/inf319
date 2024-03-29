#version 450 core

layout (location = 0) out vec3 fragmentNormal;
layout (location = 1) out vec2 fragmentTextureCoordinate;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTextureCoordinate;

struct TextureRect {
	float x, y, w, h;
};

layout (set = 0, binding = 0) uniform EntityData {
	mat4 mvp;
	mat4 normal;
	vec4 color;
	TextureRect textureRegion;
	vec2 textureScale;
} entityData;

void main() {
    gl_Position = entityData.mvp * vec4(vertexPosition, 1.0);
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
    gl_Position.y = -gl_Position.y;
    fragmentNormal = normalize(mat3(entityData.normal) * vertexNormal);
    fragmentTextureCoordinate = fract(vertexTextureCoordinate * entityData.textureScale);
	fragmentTextureCoordinate *= vec2(entityData.textureRegion.w, entityData.textureRegion.h);
	fragmentTextureCoordinate += vec2(entityData.textureRegion.x, entityData.textureRegion.y);
}
