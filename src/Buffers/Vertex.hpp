#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 colour;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		// Setup the binding description for the vertex
		VkVertexInputBindingDescription bindingDescription {};
		bindingDescription.binding	 = 0;
		bindingDescription.stride	 = sizeof( Vertex );
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		// Setup the atttribute descriptions
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {};

		// Position attribute
		attributeDescriptions[0].binding  = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format	  = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset	  = offsetof( Vertex, position );

		// Colour attribute
		attributeDescriptions[1].binding  = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format	  = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset	  = offsetof( Vertex, colour );

		// TexCoord attribute
		attributeDescriptions[2].binding  = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format	  = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset	  = offsetof( Vertex, texCoord );

		return attributeDescriptions;
	}

	bool operator==( const Vertex& other ) const
	{
		return position == other.position && colour == other.colour && texCoord == other.texCoord;
	}
};

// // clang-format off
// const std::vector<Vertex> vertices = {
// 	{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
// 	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
// 	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
// 	{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
// 	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
// 	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
// 	{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
// 	{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
// 	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
// 	{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
// 	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
// 	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
// 	{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
// 	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
// 	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
// 	{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } }
// };
// // clang-format on

// const std::vector<uint16_t> indices = {
// 	0, 2, 1, 2, 0, 3,	  // Front Face
// 	4, 5, 6, 6, 7, 4,	  // Back Face
// 	9, 10, 11, 11, 4, 9,  // Right Face
// 	13, 15, 2, 15, 13, 8, // Left Face
// 	11, 14, 5, 5, 4, 11,  // Bottom Face
// 	3, 13, 2, 13, 3, 12	  // Top Face
// };