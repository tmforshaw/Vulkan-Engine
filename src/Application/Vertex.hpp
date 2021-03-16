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

const std::vector<Vertex> vertices = {
	{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
	{ { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
	{ { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};

// Define the type of int used in the indices vector
#define INDEX_BUFFER_TYPE VK_INDEX_TYPE_UINT16 // VK_INDEX_TYPE_TYPE_UINT32