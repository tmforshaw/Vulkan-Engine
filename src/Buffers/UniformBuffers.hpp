#pragma once

#include "../Graphics/Light.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES // Ensure the data is correctly aligned
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

struct VertexUniformBufferObject
{
	alignas( 16 ) glm::mat4 model;
	alignas( 16 ) glm::mat4 view;
	alignas( 16 ) glm::mat4 proj;
};