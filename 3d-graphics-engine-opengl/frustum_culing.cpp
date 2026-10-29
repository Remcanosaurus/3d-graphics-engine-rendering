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