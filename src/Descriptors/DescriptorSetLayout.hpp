#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>

class DescriptorSetLayout
{
private:
	VkDescriptorSetLayout					  m_layout;
	std::vector<VkDescriptorSetLayoutBinding> m_bindings;

	const VkDevice* m_logicalDevice;

public:
	DescriptorSetLayout() : m_logicalDevice( nullptr ) {}

	void AddBinding( const VkDescriptorType& p_type, const uint32_t p_descriptorCount, const VkShaderStageFlags& p_stageFlags, const VkSampler* p_immutableSamplers )
	{
		// Setup the descriptor set layout binding
		VkDescriptorSetLayoutBinding binding {};
		binding.binding			   = static_cast<uint32_t>( m_bindings.size() ); // Bind to the slot it will occupy
		binding.descriptorType	   = p_type;
		binding.descriptorCount	   = p_descriptorCount;
		binding.stageFlags		   = p_stageFlags;
		binding.pImmutableSamplers = p_immutableSamplers;

		// Add to the bindings vector
		m_bindings.push_back( binding );
	}

	void CreateLayout( const VkDevice& p_logicalDevice )
	{
		// Set the member variables
		m_logicalDevice = const_cast<VkDevice*>( &p_logicalDevice );

		// Setup the descriptor set layout create information
		VkDescriptorSetLayoutCreateInfo layoutCreateInfo {};
		layoutCreateInfo.sType		  = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = static_cast<uint32_t>( m_bindings.size() );
		layoutCreateInfo.pBindings	  = m_bindings.data();

		// Create the descriptor set layout
		if ( vkCreateDescriptorSetLayout( *m_logicalDevice, &layoutCreateInfo, nullptr, &m_layout ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create descriptor set layout" );
	}

	inline const VkDescriptorSetLayout&						GetLayout() const { return m_layout; }
	inline const std::vector<VkDescriptorSetLayoutBinding>& GetBindings() const { return m_bindings; }
	inline const VkDescriptorSetLayoutBinding&				GetBinding( const uint32_t& p_index ) const { return m_bindings[p_index]; }

	void Cleanup()
	{
		// Destroy the descriptor set layout
		vkDestroyDescriptorSetLayout( *m_logicalDevice, m_layout, nullptr );
	}
};