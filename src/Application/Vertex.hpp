#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct Vertex
{
	glm::vec2 position;
	glm::vec3 colour;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		// Setup the binding description for the vertex
		VkVertexInputBindingDescription bindingDescription {};
		bindingDescription.binding	 = 0;
		bindingDescription.stride	 = sizeof( Vertex );
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
	{
		// Setup the atttribute descriptions
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {};

		// Position attribute
		attributeDescriptions[0].binding  = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format	  = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset	  = offsetof( Vertex, position );

		// Colour attribute
		attributeDescriptions[1].binding  = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format	  = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset	  = offsetof( Vertex, colour );

		return attributeDescriptions;
	}
};

#define pi 3.1415926f

#define TRIANGLE_HEIGHT sin( pi / 3 ) / 2

const std::vector<Vertex>
	vertices = {
		{ { 0.0f, -TRIANGLE_HEIGHT }, { 1.0f, 0.0f, 1.0f } },
		{ { 0.5f, TRIANGLE_HEIGHT }, { 0.0f, 1.0f, 1.0f } },
		{ { -0.5f, TRIANGLE_HEIGHT }, { 1.0f, 1.0f, 0.0f } }
	};