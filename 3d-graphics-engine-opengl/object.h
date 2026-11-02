#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include "obj_loader.h"

class Mesh
{
public:
	GLuint VAO{ 0 };

	GLuint vertexVBO{ 0 };
	GLuint uvVBO{ 0 };
	GLuint normalVBO{ 0 };
	GLuint tangentVBO{ 0 };
	GLuint bitangentVBO{ 0 };

	GLuint diffuseTexture{ 0 };
	GLuint heightMapTexture{ 0 };

	glm::vec3 ka{ 1.0f };
	glm::vec3 kd{ 1.0f };
	glm::vec3 ks{ 1.0f };
	float ns{ 48.0f };

	GLsizei vertexCount{ 0 };

	void createBuffers(
		std::vector<glm::vec3> vertices,
		std::vector<glm::vec3> uvs,
		std::vector<glm::vec3> normals,
		std::vector<glm::vec3> tangents,
		std::vector<glm::vec3> bitangents
	);

	void draw() const;
};

class Object
{
public:
	Object(ObjData objData);

	glm::vec3 getPosition() const { return position; }
	glm::vec3 getRotation() const { return rotation; }
	glm::vec3 getScale() const { return scale; }
	glm::mat4 getModelMatrix() const { return modelMatrix; }

	void setPosition(const glm::vec3& pos) { position = pos; matrixDirty = true; }
	void setRotation(const glm::vec3& rot) { rotation = rot; matrixDirty = true; }
	void setScale(const glm::vec3& s) { scale = s; matrixDirty = true; }

	ObjData objData;
	std::vector<Mesh> meshes;

	void update();

private:
	bool matrixDirty{ true };
	
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::mat4 modelMatrix{ 1.0f };
};

void generateMeshesFromObject(Object& object);