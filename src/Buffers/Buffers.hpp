#pragma once
#include "CommandBuffer.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>
#include <stdexcept>

static uint32_t FindMemoryType( const VkPhysicalDevice& p_physicalDevice, const uint32_t& typeFilter, const VkMemoryPropertyFlags& properties )
{
	// Get the GPU memory properties
	VkPhysicalDeviceMemoryProperties memProperities;
	vkGetPhysicalDeviceMemoryProperties( p_physicalDevice, &memProperities );

	// Find a suitable memory type
	for ( uint32_t i = 0; i < memProperities.memoryTypeCount; i++ )
		if ( typeFilter & ( 1 << i ) &&													  // The type of memory is suitable
			 ( memProperities.memoryTypes[i].propertyFlags & properties ) == properties ) // The memory has the correct properties
			return i;																	  // Return the index of a suitable memory type

	// No memory type was found
	throw std::runtime_error( "Failed to find suitable memory type" );
}

static void CreateBuffer( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkDeviceSize& p_size, const VkBufferUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties, VkBuffer* p_buffer, VkDeviceMemory* p_bufferMemory )
{
	// Setup the create information for the buffer
	VkBufferCreateInfo bufferCreateInfo {};
	bufferCreateInfo.sType		 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size		 = p_size;
	bufferCreateInfo.usage		 = p_usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.flags		 = 0;

	// Create the buffer
	if ( vkCreateBuffer( p_logicalDevice, &bufferCreateInfo, nullptr, p_buffer ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to create buffer" );

	// Setup the memory requirements for the buffer
	VkMemoryRequirements memRequirements {};
	vkGetBufferMemoryRequirements( p_logicalDevice, *p_buffer, &memRequirements );

	// Setup the allocation information for the buffer
	VkMemoryAllocateInfo bufferAllocInfo {};
	bufferAllocInfo.sType			= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferAllocInfo.allocationSize	= memRequirements.size;
	bufferAllocInfo.memoryTypeIndex = FindMemoryType( p_physicalDevice, memRequirements.memoryTypeBits, p_properties ); // Find a memory type with the correct properties

	// Allocate the memory of the buffer
	if ( vkAllocateMemory( p_logicalDevice, &bufferAllocInfo, nullptr, p_bufferMemory ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to allocate buffer memory" );

	// Associate the memory with the buffer
	vkBindBufferMemory( p_logicalDevice, *p_buffer, *p_bufferMemory, 0 );
}

static void CopyBuffer( const VkDevice& p_logicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize& p_size )
{
	// Create a one-time command buffer
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands( p_logicalDevice, p_commandPool );

	// Setup the copy region
	VkBufferCopy copyRegion {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size		 = p_size;

	// Create the copy region
	vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

	// End the command buffer recording and free the memory
	EndSingleTimeCommands( p_logicalDevice, p_graphicsQueue, p_commandPool, commandBuffer );
}

static void UpdateBufferViaStagingBuffer( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkDeviceSize& p_size, const void* p_data, VkBuffer* p_buffer )
{
	// Setup the staging buffer
	VkBuffer	   stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// Create a staging buffer
	CreateBuffer( p_logicalDevice, p_physicalDevice, p_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory );

	// Fill the staging buffer with data (Map the buffer memory into CPU accessible memory)
	void* mappedMemPtr;
	vkMapMemory( p_logicalDevice, stagingBufferMemory, 0, p_size, 0, &mappedMemPtr );

	// Copy the vertices into the memory address
	std::memcpy( mappedMemPtr, p_data, (size_t)p_size );

	// Remove the mapping to CPU accessible memory
	vkUnmapMemory( p_logicalDevice, stagingBufferMemory );

	// Copy the data from the staging buffer to the buffer
	CopyBuffer( p_logicalDevice, p_commandPool, p_graphicsQueue, stagingBuffer, *p_buffer, p_size );

	// Destroy the staging buffer and free it's memory
	vkDestroyBuffer( p_logicalDevice, stagingBuffer, nullptr );
	vkFreeMemory( p_logicalDevice, stagingBufferMemory, nullptr );
}

static void CreateBufferViaStagingBuffer( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkDeviceSize& p_size, const void* p_data, const VkBufferUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties, VkBuffer* p_buffer, VkDeviceMemory* p_bufferMemory )
{
	// Create the buffer
	CreateBuffer( p_logicalDevice, p_physicalDevice, p_size, p_usage, p_properties, p_buffer, p_bufferMemory );

	// Update the buffer using a staging buffer
	UpdateBufferViaStagingBuffer( p_logicalDevice, p_physicalDevice, p_commandPool, p_graphicsQueue, p_size, p_data, p_buffer );
}