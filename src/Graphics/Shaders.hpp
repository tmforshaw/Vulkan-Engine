#pragma once
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static std::vector<char> ReadFile( const std::string& filepath )
{
	// Create a filestream that starts reading from the end, and treats it as binary
	std::ifstream fs( filepath, std::ios::ate | std::ios::binary );

	// Create a buffer
	std::vector<char> buffer;

	if ( fs.is_open() )
	{
		// Get the position (starting at the end gets the size)
		size_t fileSize = (size_t)fs.tellg();

		// Set the buffer size
		buffer = std::vector<char>( fileSize );

		// Go to the beginning of the files
		fs.seekg( 0 );
		fs.read( buffer.data(), fileSize );

		// Close the filestream
		fs.close();
	}
	else
		throw std::runtime_error( "Failed to open file" );

	return buffer;
}

static VkShaderModule CreateShaderModule( const std::vector<char>& code, const VkDevice& p_device )
{
	// Setup the create information for the module
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode	= reinterpret_cast<const uint32_t*>( code.data() );

	VkShaderModule shaderModule;
	if ( vkCreateShaderModule( p_device, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to create shader module" );

	return shaderModule;
}