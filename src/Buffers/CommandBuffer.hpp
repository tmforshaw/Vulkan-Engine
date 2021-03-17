#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>

static VkCommandBuffer BeginSingleTimeCommands( const VkDevice& p_logicalDevice, const VkCommandPool& p_commandPool )
{
	// Setup the allocation information for the command buffer
	VkCommandBufferAllocateInfo commandBufferAllocInfo {};
	commandBufferAllocInfo.sType			  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocInfo.commandPool		  = p_commandPool;
	commandBufferAllocInfo.level			  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocInfo.commandBufferCount = 1;

	// Create the command buffers
	VkCommandBuffer commandBuffer;
	if ( vkAllocateCommandBuffers( p_logicalDevice, &commandBufferAllocInfo, &commandBuffer ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to allocate command buffers" );

	// Setup the begin information for the command buffer
	VkCommandBufferBeginInfo commandBufferBeginInfo {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	// Create the command buffer
	if ( vkBeginCommandBuffer( commandBuffer, &commandBufferBeginInfo ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to begin recording to command buffer" );

	return commandBuffer;
}

static void EndSingleTimeCommands( const VkDevice& p_logicalDevice, const VkQueue& p_graphicsQueue, const VkCommandPool& p_commandPool, const VkCommandBuffer& p_commandBuffer )
{
	// Finish the recording and check for errors
	if ( vkEndCommandBuffer( p_commandBuffer ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to record command buffer" );

	// Submit the command buffer
	VkSubmitInfo submitInfo {};
	submitInfo.sType			  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers	  = &p_commandBuffer;

	// Execute the command buffer to complete the transfer of data
	if ( vkQueueSubmit( p_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to submit draw command buffer" );

	// Wait until data is transferred
	vkQueueWaitIdle( p_graphicsQueue );

	// Free the command buffer
	vkFreeCommandBuffers( p_logicalDevice, p_commandPool, 1, &p_commandBuffer );
}