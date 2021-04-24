#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
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
		newDescription.location = static_cast<uint32_t>( attributeDescriptions.size() );
		newDescription.format	= VK_FORMAT_R32G32B32_SFLOAT;
		newDescription.offset	= offsetof( Vertex, position );

		// Add description to vector
		attributeDescriptions.push_back( newDescription );

		// Normal attribute
		newDescription.binding	= 0;
		newDescription.location = static_cast<uint32_t>( attributeDescriptions.size() );
		newDescription.format	= VK_FORMAT_R32G32B32_SFLOAT;
		newDescription.offset	= offsetof( Vertex, normal );

		// Add description to vector
		attributeDescriptions.push_back( newDescription );

		// TexCoord attribute
		newDescription.binding	= 0;
		newDescription.location = static_cast<uint32_t>( attributeDescriptions.size() );
		newDescription.format	= VK_FORMAT_R32G32_SFLOAT;
		newDescription.offset	= offsetof( Vertex, texCoord );

		// Add description to vector
		attributeDescriptions.push_back( newDescription );

		// SamplerID attribute
		newDescription.binding	= 0;
		newDescription.location = static_cast<uint32_t>( attributeDescriptions.size() );
		newDescription.format	= VK_FORMAT_R32_UINT;
		newDescription.offset	= offsetof( Vertex, samplerID );

		// Add description to vector
		attributeDescriptions.push_back( newDescription );

		return attributeDescriptions;
	}

	bool operator==( const Vertex& other ) const
	{
		return position == other.position && texCoord == other.texCoord && normal == other.normal && samplerID == other.samplerID;
	}
};