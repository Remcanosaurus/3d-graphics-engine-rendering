#pragma once
#include <glm.hpp>
struct Plane
{
	glm::vec3 normal{ 0.0f, 1.0f, 0.0f };
	float distance{ 0.0f };
};

struct Frustum
{
	Plane top;
	Plane bottom;
	Plane right;
	Plane left;
	Plane near;
	Plane far;
};