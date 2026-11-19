#pragma once
#include <string>
#include <vector>
#include <glm.hpp>

struct Texture
{
	int width{ 0 };
	int height{ 0 };

	std::vector<uint32_t> pixels;

	bool isEmpty() const { return pixels.empty(); }
};

struct Material
{
	std::string name;

	glm::vec3 ambientColor{ 1.0f };
	glm::vec3 diffuseColor{ 1.0f };
	glm::vec3 specularColor{ 1.0f };
	glm::vec3 transmissionFilter{ 1.0f };

	float shininess{ 0.0f };
	float opticalDensity{ 1.0f };

	float transparency{ 1.0f };

	Texture ambientTexture;
	Texture diffuseTexture;
	Texture specularTexture;
	Texture shininessTexture;
	Texture transparencyTexture;
	Texture bumpTexture;
};

struct ObjFace
{
	std::vector<int> vertexIndices;
	std::vector<int> textureCoordIndices;
	std::vector<int> normalIndices;

	glm::vec3 tangents[3]{ glm::vec3(0.0f)};
	glm::vec3 bitangents[3]{ glm::vec3(0.0f)};

	int materialIndex{ -1 };
};


struct ObjData
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> textureCoords;
	std::vector<glm::vec3> normals;

	std::vector<ObjFace> faces;

	std::vector<Material> materials;
};

struct TangentKey
{
	int vertex;
	int normal;
	int uv;

	bool operator==(const TangentKey& other) const
	{
		return vertex == other.vertex &&
			normal == other.normal &&
			uv == other.uv;
	}
};

struct TangentKeyHash
{
	std::size_t operator()(const TangentKey& key) const
	{
		std::size_t h1 = std::hash<int>{}(key.vertex);
		std::size_t h2 = std::hash<int>{}(key.normal);
		std::size_t h3 = std::hash<int>{}(key.uv);

		return h1 ^ (h2 << 1) ^ (h3 << 2);
	}
};

struct TangentAccumulation
{
	glm::vec3 tangent{ 0.0f };
	glm::vec3 bitangent{ 0.0f };
};

Texture loadTexture(const std::string& filename);

void loadMtlFile(const std::string& filename, std::vector<Material>& materials);

ObjData loadObjFile(const std::string& filename);

void calculateTangents(ObjData& objData);