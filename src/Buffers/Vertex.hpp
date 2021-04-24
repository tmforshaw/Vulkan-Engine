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
	uint32_t  samplerID;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		// Setup the binding description for the vertex
		VkVertexInputBindingDescription bindingDescription {};
		bindingDescription.binding	 = 0;
		bindingDescription.stride	 = sizeof( Vertex );
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
	{
		// Setup the atttribute descriptions
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions {};
		VkVertexInputAttributeDescription			   newDescription {};

		// Position attribute
		newDescription.binding	= 0;
		newDescription.location = 0;
		newDescription.format	= VK_FORMAT_R32G32B32_SFLOAT;
		newDescription.offset	= offsetof( Vertex, position );

		// Add description to vector
		attributeDescriptions.push_back( newDescription );

		// Colour attribute
		newDescription.binding	= 0;
		newDescription.location = 1;
		newDescription.format	= VK_FORMAT_R32G32B32_SFLOAT;
		newDescription.offset	= offsetof( Vertex, colour );

		// Add description to vector
		attributeDescriptions.push_back( newDescription );

		// TexCoord attribute
		newDescription.binding	= 0;
		newDescription.location = 2;
		newDescription.format	= VK_FORMAT_R32G32_SFLOAT;
		newDescription.offset	= offsetof( Vertex, texCoord );

		// Add description to vector
		attributeDescriptions.push_back( newDescription );

		// SamplerID attribute
		newDescription.binding	= 0;
		newDescription.location = 3;
		newDescription.format	= VK_FORMAT_R32_UINT;
		newDescription.offset	= offsetof( Vertex, samplerID );

		// Add description to vector
		attributeDescriptions.push_back( newDescription );

		return attributeDescriptions;
	}

	bool operator==( const Vertex& other ) const
	{
		return position == other.position && colour == other.colour && texCoord == other.texCoord && samplerID == other.samplerID;
	}
};