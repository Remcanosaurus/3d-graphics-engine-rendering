#include "lod.h"
#include<algorithm>
#include <queue>

SimObject createSimObject(const ObjData& objData)
{
	SimObject simObject;
	simObject.vertices.resize(objData.vertices.size());
	for (size_t i = 0; i < objData.vertices.size(); ++i)
	{
		simObject.vertices[i].position = objData.vertices[i];
		simObject.vertices[i].quadric = glm::mat4(0.0f);
	}

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

	return simObject;
}

bool edgeComparator(const SimEdge& a, const SimEdge& b)
{
	return a.error > b.error;
}

SimObject simplifyObject(SimObject simObject, float targetReduction)
{
	size_t targetFaceCount = static_cast<size_t>(simObject.faces.size() * (1.0f - targetReduction));

	while (simObject.faces.size() > targetFaceCount)
	{
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

				if (std::abs(glm::determinant(q)) > 1e-8f)
				{
					glm::mat4 A(
						q[0][0], q[0][1], q[0][2], q[0][3],
						q[0][1], q[1][1], q[1][2], q[1][3],
						q[0][2], q[1][2], q[2][2], q[2][3],
						0.0f, 0.0f, 0.0f, 1.0f
					);

					glm::vec4 v = glm::inverse(A) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
					edge.optimalPosition = glm::vec3(v.x, v.y, v.z);
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
	}
}