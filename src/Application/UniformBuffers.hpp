#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES // Ensure the data is correctly aligned
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct UniformBufferObject
{
	alignas( 16 ) glm::mat4 model;
	alignas( 16 ) glm::mat4 view;
	alignas( 16 ) glm::mat4 proj;
};