#pragma once
#include <glm.hpp>
struct Plane
{
	glm::vec3 normal{ 0.0f, 1.0f, 0.0f };
	float distance{ 0.0f };

	Plane() = default;

	Plane(const glm::vec3& point, const glm::vec3& normal)
		: normal{ normal },
		distance{ glm::dot(normal,point) }
	{}

	float getSignedDistanceToPlane(const glm::vec3& point) const
	{
		return glm::dot(normal, point) - distance;
	}
};

struct AABB
{
	glm::vec3 center{ 0.0f };
	glm::vec3 extents{ 0.0f };

	AABB(const glm::vec3& min, const glm::vec3& max)
		: center{ (max + min ) * 0.5f },
		extents{ max.x - center.x, max.y - center.y, max.z - center.z }
	{}

	AABB(const glm::vec3& boxCenter, float halfWidth, float halfHeight, float halfDepth)
		: center{ boxCenter },
		extents{ halfWidth, halfHeight, halfDepth }
	{}
};

struct Frustum
{
	Plane top;
	Plane bottom;
	Plane right;
	Plane left;
	Plane near;
	Plane far;

	bool intersects(const AABB& bounds);
};

Frustum calculateCameraFrustum(glm::vec3 cameraPosition, glm::vec3 cameraFront, glm::vec3 cameraRight, glm::vec3 cameraUp, float aspect, float fovY, float near, float far);