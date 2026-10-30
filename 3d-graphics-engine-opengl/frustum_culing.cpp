#include "frustum_culling.h"

Frustum calculateCameraFrustum(glm::vec3 cameraPosition, glm::vec3 cameraFront, glm::vec3 cameraRight, glm::vec3 cameraUp, float aspect, float fovY, float near, float far)
{
	Frustum frustum;
	float halfWidth = far * tanf(fovY * 0.5);
	float halfHeight = halfWidth * aspect;
	glm::vec3 farPosFromCam = far * cameraFront;

	frustum.near = { cameraPosition + near * cameraFront, cameraFront};
	frustum.far = { cameraPosition + farPosFromCam, -cameraFront};
	frustum.right = { cameraPosition, glm::cross(farPosFromCam - cameraRight * halfHeight, cameraUp) };
	frustum.left = { cameraPosition, glm::cross(cameraUp, farPosFromCam + cameraRight * halfHeight) };
	frustum.top = { cameraPosition, glm::cross(cameraRight, farPosFromCam - cameraUp * halfWidth) };
	frustum.bottom = { cameraPosition, glm::cross(farPosFromCam + cameraUp * halfWidth, cameraRight) };

	return frustum;
}

AABB calculateWorldAABB(const std::vector<glm::vec3>& vertices, const glm::mat4& model)
{
	glm::vec3 min = vertices[0];
	glm::vec3 max = vertices[0];

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		min = glm::min(min, vertices[i]);
		max = glm::max(max, vertices[i]);
	}

	glm::vec3 center = (min + max) * 0.5f;
	center = glm::vec3(model * glm::vec4(center, 1.0f));

	glm::mat3 rotationScale{ model };

	glm::vec3 extents = (max - min) * 0.5f;
	extents =
		glm::abs(rotationScale[0]) * extents.x +
		glm::abs(rotationScale[1]) * extents.y +
		glm::abs(rotationScale[2]) * extents.z;
	
	return AABB(center, extents.x, extents.y, extents.z);
}