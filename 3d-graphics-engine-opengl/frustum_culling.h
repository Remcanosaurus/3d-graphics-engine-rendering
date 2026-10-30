#pragma once
#include <glm.hpp>
#include "obj_loader.h"

struct AABB
{
	glm::vec3 center{ 0.0f };
	glm::vec3 extents{ 0.0f };

	AABB(const glm::vec3& boxCenter, float halfWidth, float halfHeight, float halfDepth)
		: center{ boxCenter },
		extents{ halfWidth, halfHeight, halfDepth }
	{}
};

struct Plane
{
	glm::vec3 normal{ 0.0f, 1.0f, 0.0f };
	float distance{ 0.0f };

	Plane() = default;

	Plane(const glm::vec3& point, const glm::vec3& normal)
		: normal{ normal },
		distance{ glm::dot(normal,point) }
	{}

	float getSignedDistance(const glm::vec3& point) const
	{
		return glm::dot(normal, point) - distance;
	}

	bool isOnOrForward(const AABB& bounds) const
	{
		float r =
			bounds.extents.x * std::abs(normal.x) + 
			bounds.extents.y * std::abs(normal.y) + 
			bounds.extents.z * std::abs(normal.z);
		
		return -r <= getSignedDistance(bounds.center);
	}
};

struct Frustum
{
	Plane top;
	Plane bottom;
	Plane right;
	Plane left;
	Plane near;
	Plane far;

	bool isInsideFrustum(const AABB& bounds) const
	{
		return 
			top.isOnOrForward(bounds) && bottom.isOnOrForward(bounds) && 
			right.isOnOrForward(bounds) && left.isOnOrForward(bounds) && 
			near.isOnOrForward(bounds) && far.isOnOrForward(bounds);
	}
};

Frustum calculateCameraFrustum(glm::vec3 cameraPosition, glm::vec3 cameraFront, glm::vec3 cameraRight, glm::vec3 cameraUp, float aspect, float fovY, float near, float far);

AABB calculateWorldAABB(const std::vector<glm::vec3>& vertices, const glm::mat4& model);