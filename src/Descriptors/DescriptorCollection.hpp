#pragma once

#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class DescriptorCollection
{
private:
	DescriptorSetLayout				  m_layout;
	DescriptorPool					  m_pool;
	std::vector<VkWriteDescriptorSet> m_writes; // Writes for each set (reset after every set)
	std::vector<VkDescriptorSet>	  m_sets;
	uint32_t						  m_size;

	std::vector<VkDescriptorBufferInfo> m_bufferInfos; // BufferInfo for each set (reset after every set)
	std::vector<VkDescriptorImageInfo>	m_imageInfos;  // ImageInfo for each set (reset after every set)

	const VkDevice* m_logicalDevice;

public:
	DescriptorCollection() : m_logicalDevice( nullptr ) {}

	void Init( const VkDevice& p_logicalDevice, const uint32_t& p_size )
	{
		// Set member variables
		m_logicalDevice = const_cast<VkDevice*>( &p_logicalDevice );
		m_size			= p_size;
	}

	void AddLayoutBinding( const VkDescriptorType& p_type, const uint32_t p_descriptorCount, const VkShaderStageFlags& p_stageFlags, const VkSampler* p_immutableSamplers )
	{
		m_layout.AddBinding( p_type, p_descriptorCount, p_stageFlags, p_immutableSamplers );
	}

	void CreateLayout()
	{
		m_layout.CreateLayout( *m_logicalDevice );
	}

	void CreatePool( const VkDescriptorPoolCreateFlags& p_flags )
	{
		// Get the bindings from the layout
		std::vector<VkDescriptorSetLayoutBinding> bindings = m_layout.GetBindings();

		// Initialise the descriptor pool
		m_pool.Init( *m_logicalDevice );

		// Add all of the correct sizes to the pool
		for ( const auto& binding : bindings )
			m_pool.AddSize( binding.descriptorType, m_size );

		// Create the descriptor pool
		m_pool.CreatePool( m_size, p_flags );
	}

	void InitSets()
	{
		// Create a vector of of descriptor set layouts and fill with m_descriptorSetLayout
		std::vector<VkDescriptorSetLayout> layouts( m_size, m_layout.GetLayout() );

		// Setup the allocation information for the descriptor sets
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo {};
		descriptorSetAllocInfo.sType			  = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.descriptorPool	  = m_pool.GetPool();
		descriptorSetAllocInfo.descriptorSetCount = m_size;
		descriptorSetAllocInfo.pSetLayouts		  = layouts.data();

		// Clear and resize the descriptor set vector
		m_sets = {};
		m_sets.resize( m_size );

		// Clear other vectors
		m_writes	  = {};
		m_bufferInfos = {};
		m_imageInfos  = {};

		// Allocate the descriptor sets
		if ( vkAllocateDescriptorSets( *m_logicalDevice, &descriptorSetAllocInfo, m_sets.data() ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to allocate descriptor sets" );
	}

	void AddBuffer( const VkBuffer& p_buffer, const VkDeviceSize& p_offset, const uint32_t& p_size, const VkDescriptorType& p_type )
	{
		// Create a new write
		VkWriteDescriptorSet newWrite {};

		// Configure the descriptors using buffer information
		VkDescriptorBufferInfo bufferInfo {};
		bufferInfo.buffer = p_buffer;
		bufferInfo.offset = p_offset;
		bufferInfo.range  = p_size;

		// Add the buffer info to the vector
		m_bufferInfos.push_back( bufferInfo );

		// Configure the uniform buffer write descriptor set
		newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// newWrite.dstSet			  = m_sets[i];								// Leave undefined
		newWrite.dstBinding		  = static_cast<uint32_t>( m_writes.size() ); // The index of this write
		newWrite.dstArrayElement  = 0;
		newWrite.descriptorType	  = p_type;
		newWrite.descriptorCount  = 1;
		newWrite.pBufferInfo	  = &m_bufferInfos[m_bufferInfos.size() - 1];
		newWrite.pImageInfo		  = nullptr;
		newWrite.pTexelBufferView = nullptr;

		// Add to the writes array
		m_writes.push_back( newWrite );
	}

	void AddImage( const VkImageLayout& p_imageLayout, const VkImageView& p_imageView, const VkSampler& p_sampler, const VkDescriptorType& p_type )
	{
		// Create a new write
		VkWriteDescriptorSet newWrite {};

		// Configure the descriptors using image information
		VkDescriptorImageInfo imageInfo {};
		imageInfo.imageLayout = p_imageLayout;
		imageInfo.imageView	  = p_imageView;
		imageInfo.sampler	  = p_sampler;

		// Add the image info to the vector
		m_imageInfos.push_back( imageInfo );

		// Configure the sampler write descriptor set
		newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// newWrite.dstSet			 = m_sets[i];							   // Leave undefined
		newWrite.dstBinding		  = static_cast<uint32_t>( m_writes.size() ); // The index of this write
		newWrite.dstArrayElement  = 0;
		newWrite.descriptorType	  = p_type;
		newWrite.descriptorCount  = 1;
		newWrite.pBufferInfo	  = nullptr;
		newWrite.pImageInfo		  = &m_imageInfos[m_imageInfos.size() - 1];
		newWrite.pTexelBufferView = nullptr;

		// Add to the writes array
		m_writes.push_back( newWrite );
	}

	void UpdateSet( const uint32_t& p_index )
	{
		for ( auto& write : m_writes ) // Set the dstSet
			write.dstSet = m_sets[p_index];

		// Update the descriptor sets
		vkUpdateDescriptorSets( *m_logicalDevice, static_cast<uint32_t>( m_writes.size() ), m_writes.data(), 0, nullptr );

		// Clear the vectors
		m_writes	  = {};
		m_bufferInfos = {};
		m_imageInfos  = {};
	}

	inline const std::vector<VkDescriptorSet>& GetSets() const { return m_sets; }
	inline const VkDescriptorSet&			   GetSet( const uint32_t& p_index ) const { return m_sets[p_index]; }
	inline const VkDescriptorSet*			   GetSetRef( const uint32_t& p_index ) const { return &m_sets[p_index]; }
	inline const VkDescriptorSetLayout&		   GetLayout() const { return m_layout.GetLayout(); }

	void CleanupPool()
	{
		m_pool.Cleanup();
	}

	void CleanupLayout()
	{
		m_layout.Cleanup();
	}
};