#include "lod.h"
#include<algorithm>
#include <queue>
#include <iostream>

SimObject createSimObject(const ObjData& objData)
{
	std::cout << "Creating SimObject from ObjData with " << objData.vertices.size() << " vertices and " << objData.faces.size() << " faces.\n";
	SimObject simObject;
	simObject.vertices.resize(objData.vertices.size());
	for (size_t i = 0; i < objData.vertices.size(); ++i)
	{
		simObject.vertices[i].position = objData.vertices[i];
		simObject.vertices[i].quadric = glm::mat4(0.0f);
	}

	std::cout << "Initialized " << simObject.vertices.size() << " SimVertices.\n";

	simObject.faces = objData.faces;

	for (size_t i = 0; i < objData.faces.size(); ++i)
	{
		const ObjFace& face = objData.faces[i];

		for (size_t j = 0; j < 3; ++j)
		{
			int current = face.vertexIndices[j];
			int next = face.vertexIndices[(j + 1) % 3];
			int previous = face.vertexIndices[(j + 2) % 3];

			simObject.vertices[current].connectedFaces.push_back(static_cast<int>(i));
				
			simObject.vertices[current].connectedVertices.push_back(next);
			simObject.vertices[current].connectedVertices.push_back(previous);
		}
	}

	std::cout << "Calculated connectivity for " << simObject.vertices.size() << " vertices.\n";

	for (SimVertex& vertex : simObject.vertices)
	{
		std::sort(vertex.connectedVertices.begin(), vertex.connectedVertices.end());
		vertex.connectedVertices.erase(std::unique(vertex.connectedVertices.begin(), vertex.connectedVertices.end()), vertex.connectedVertices.end());

		std::sort(vertex.connectedFaces.begin(), vertex.connectedFaces.end());
		vertex.connectedFaces.erase(std::unique(vertex.connectedFaces.begin(), vertex.connectedFaces.end()), vertex.connectedFaces.end());
	}

	for (const ObjFace& face : objData.faces)
	{
		glm::vec3 v0 = simObject.vertices[face.vertexIndices[0]].position;
		glm::vec3 v1 = simObject.vertices[face.vertexIndices[1]].position;
		glm::vec3 v2 = simObject.vertices[face.vertexIndices[2]].position;

		glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
		float d = -glm::dot(normal, v0);

		glm::vec4 plane(normal, d);
		glm::mat4 kp = glm::outerProduct(plane, plane);

		for (int vertexIndex : face.vertexIndices)
		{
			simObject.vertices[vertexIndex].quadric += kp;
		}
	}

	std::cout << "Calculated quadrics for " << simObject.vertices.size() << " vertices.\n";

	return simObject;
}

bool edgeComparator(const SimEdge& a, const SimEdge& b)
{
	return a.error > b.error;
}

void rebuildConnectivity(SimObject& simObject)
{
	for (SimVertex& vertex : simObject.vertices)
	{
		vertex.connectedFaces.clear();
		vertex.connectedVertices.clear();
	}

	for (size_t faceIndex = 0; faceIndex < simObject.faces.size(); ++faceIndex)
	{
		const ObjFace& face = simObject.faces[faceIndex];
		if (face.vertexIndices.size() < 3)
			continue;

		for (size_t i = 0; i < 3; ++i)
		{
			int current = face.vertexIndices[i];
			int next = face.vertexIndices[(i + 1) % 3];
			int previous = face.vertexIndices[(i + 2) % 3];

			if (current < 0 || next < 0 || previous < 0 ||
				current >= static_cast<int>(simObject.vertices.size()) ||
				next >= static_cast<int>(simObject.vertices.size()) ||
				previous >= static_cast<int>(simObject.vertices.size()))
			{
				continue;
			}

			simObject.vertices[current].connectedFaces.push_back(static_cast<int>(faceIndex));
			simObject.vertices[current].connectedVertices.push_back(next);
			simObject.vertices[current].connectedVertices.push_back(previous);
		}
	}

	for (SimVertex& vertex : simObject.vertices)
	{
		std::sort(vertex.connectedVertices.begin(), vertex.connectedVertices.end());
		vertex.connectedVertices.erase(std::unique(vertex.connectedVertices.begin(), vertex.connectedVertices.end()), vertex.connectedVertices.end());

		std::sort(vertex.connectedFaces.begin(), vertex.connectedFaces.end());
		vertex.connectedFaces.erase(std::unique(vertex.connectedFaces.begin(), vertex.connectedFaces.end()), vertex.connectedFaces.end());
	}
}

SimObject simplifyObject(SimObject simObject, float targetReduction)
{
	if (targetReduction == 1E-8f)
		return simObject;
	size_t targetFaceCount = static_cast<size_t>(simObject.faces.size() * (1.0f - targetReduction));

	while (simObject.faces.size() > targetFaceCount)
	{
		if (simObject.faces.size() % 100 < 5) std::cout << "Current face count: " << simObject.faces.size() << ", target: " << targetFaceCount << "\n";
		rebuildConnectivity(simObject);

		std::priority_queue<SimEdge, std::vector<SimEdge>, decltype(&edgeComparator)> edges(edgeComparator);
		for (uint32_t i = 0; i < simObject.vertices.size(); ++i)
		{
			if (simObject.vertices[i].deleted)
				continue;

			for (uint32_t neighbor : simObject.vertices[i].connectedVertices)
			{
				if (simObject.vertices[neighbor].deleted || i >= neighbor)
					continue;

				SimEdge edge;
				edge.v1 = i;
				edge.v2 = neighbor;

				glm::mat4 q = simObject.vertices[i].quadric + simObject.vertices[neighbor].quadric;

				glm::mat3 A(q[0], q[1], q[2]);
				glm::vec3 b(q[3]);

				if (std::abs(glm::determinant(A)) > 1e-8f)
				{
					edge.optimalPosition = -(glm::inverse(A) * b);

					glm::vec4 v(edge.optimalPosition, 1.0f);
					edge.error = glm::dot(v, q * v);
				}
				else
				{
					for (int check = 0; check < 5; ++check)
					{
						glm::vec3 candidate = glm::mix(simObject.vertices[i].position, simObject.vertices[neighbor].position, (float)check / 4.0f);
						float error = glm::dot(glm::vec4(candidate, 1.0f), q * glm::vec4(candidate, 1.0f));

						if (error < edge.error)
						{
							edge.error = error;
							edge.optimalPosition = candidate;
						}
					}
				}

				edges.emplace(edge);
			}
		}

		if (edges.empty())
			break;

		SimEdge bestEdge = edges.top();

		SimVertex& v1 = simObject.vertices[bestEdge.v1];
		SimVertex& v2 = simObject.vertices[bestEdge.v2];

		v1.position = bestEdge.optimalPosition;
		v1.quadric += v2.quadric;

		for (int faceIndex : v2.connectedFaces)
		{
			ObjFace& face = simObject.faces[faceIndex];
			for (int& vertexIndex : face.vertexIndices)
			{
				if (vertexIndex == bestEdge.v2)
					vertexIndex = bestEdge.v1;
			}
		}

		simObject.faces.erase(std::remove_if(simObject.faces.begin(), simObject.faces.end(), [&](const ObjFace& face) {
			return face.vertexIndices[0] == face.vertexIndices[1] ||
				face.vertexIndices[1] == face.vertexIndices[2] ||
				face.vertexIndices[2] == face.vertexIndices[0]; }), simObject.faces.end());

		v2.deleted = true;
	}

	return simObject;
}

ObjData convertSimObjectToObjData(const SimObject& simObj, const ObjData& originalTemplate)
{
	ObjData newObjData;
	newObjData.materials = originalTemplate.materials;

	std::vector<int> oldToNew(simObj.vertices.size(), -1);
	int newIndex = 0;

	for (size_t i = 0; i < simObj.vertices.size(); ++i)
	{
		if (!simObj.vertices[i].deleted)
		{
			oldToNew[i] = newIndex++;
			newObjData.vertices.push_back(simObj.vertices[i].position);
		}
	}
	newObjData.textureCoords = originalTemplate.textureCoords;
	newObjData.normals.resize(newObjData.vertices.size(), glm::vec3(0.0f));

	for (const auto& face : simObj.faces)
	{
		int v0 = oldToNew[face.vertexIndices[0]];
		int v1 = oldToNew[face.vertexIndices[1]];
		int v2 = oldToNew[face.vertexIndices[2]];

		if (v0 == -1 || v1 == -1 || v2 == -1 || v0 == v1 || v1 == v2 || v2 == v0)
			continue;

		ObjFace newFace;

		newFace.vertexIndices = { v0, v1, v2 };
		newFace.materialIndex = face.materialIndex;

		if (!originalTemplate.textureCoords.empty() && !face.textureCoordIndices.empty() && face.textureCoordIndices.size() >= 3)
		{
			newFace.textureCoordIndices = {face.textureCoordIndices[0], face.textureCoordIndices[1], face.textureCoordIndices[2]};
		}
		newFace.normalIndices = { v0, v1, v2 };

		newObjData.faces.push_back(newFace);
	}

	for (const auto& face : newObjData.faces)
	{
		glm::vec3 p0 = newObjData.vertices[face.vertexIndices[0]];
		glm::vec3 p1 = newObjData.vertices[face.vertexIndices[1]];
		glm::vec3 p2 = newObjData.vertices[face.vertexIndices[2]];

		glm::vec3 faceNormal = glm::normalize(glm::cross(p1 - p0, p2 - p0));

		newObjData.normals[face.vertexIndices[0]] += faceNormal;
		newObjData.normals[face.vertexIndices[1]] += faceNormal;
		newObjData.normals[face.vertexIndices[2]] += faceNormal;
	}

	for (auto& n : newObjData.normals)
	{
		if (glm::length(n) > 0.0f)
			n = glm::normalize(n);
	}

	if (!newObjData.vertices.empty() && !newObjData.faces.empty())
	{
		calculateTangents(newObjData);
	}

	return newObjData;
}