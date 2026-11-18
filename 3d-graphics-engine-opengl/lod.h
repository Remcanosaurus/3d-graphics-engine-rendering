#pragma once
#include <glm.hpp>
#include "obj_loader.h"

struct SimEdge
{
	uint32_t v1;
	uint32_t v2;

	glm::vec3 optimalPosition{ 0.0f };
	float error{ FLT_MAX };
};

struct SimVertex
{
	glm::vec3 position{ 0.0f };
	glm::mat4 quadric{ 0.0f };

	std::vector<int> connectedFaces;
	std::vector<int> connectedVertices;

	bool deleted{ false };
};

struct SimObject
{
	std::vector<SimVertex> vertices;
	std::vector<ObjFace> faces;
};

SimObject createSimObject(const ObjData& objData);

SimObject simplifyObject(SimObject simObject, float targetReduction);

ObjData convertSimObjectToObjData(const SimObject& simObject, const ObjData& originalData);