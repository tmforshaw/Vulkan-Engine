#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <vector>

class DescriptorPool
{
private:
	std::vector<VkDescriptorPoolSize> m_poolSizes;
	VkDescriptorPool				  m_pool;

	const VkDevice* m_logicalDevice;

public:
	DescriptorPool() : m_logicalDevice( nullptr ) {}

	void Init( const VkDevice& p_logicalDevice )
	{
		// Set the member variables
		m_logicalDevice = const_cast<VkDevice*>( &p_logicalDevice );

		// Reset the pool sizes
		m_poolSizes = {};
	}

	void AddSize( const VkDescriptorType& p_type, const uint32_t& p_descriptorCount )
	{
		// Create the pool size object
		VkDescriptorPoolSize poolSize {};
		poolSize.type			 = p_type;
		poolSize.descriptorCount = p_descriptorCount;

		// Add it to the pool sizes vector
		m_poolSizes.push_back( poolSize );

		// std::cout << ( p_descriptorCount ) << std::endl;
	}

	void CreatePool( const uint32_t& p_maxSets, const VkDescriptorPoolCreateFlags& p_flags )
	{
		// Setup the create information for the decriptor pool
		VkDescriptorPoolCreateInfo poolCreateInfo {};
		poolCreateInfo.sType		 = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = static_cast<uint32_t>( m_poolSizes.size() );
		poolCreateInfo.pPoolSizes	 = m_poolSizes.data();
		poolCreateInfo.maxSets		 = p_maxSets;
		poolCreateInfo.flags		 = p_flags;

		// Create the descriptor pool
		if ( vkCreateDescriptorPool( *m_logicalDevice, &poolCreateInfo, nullptr, &m_pool ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create descriptor pool" );
	}

	inline const VkDescriptorPool& GetPool() const { return m_pool; }

	void Cleanup()
	{
		// Destroy the descriptor pool
		vkDestroyDescriptorPool( *m_logicalDevice, m_pool, nullptr );
	}
};