#pragma once

#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <tuple>
#include <vector>

class DescriptorCollection
{
private:
	DescriptorSetLayout			 m_layout;
	DescriptorPool				 m_pool;
	std::vector<VkDescriptorSet> m_sets;
	uint32_t					 m_size;

	std::vector<std::tuple<const std::vector<VkDescriptorBufferInfo>, const VkDescriptorType>> m_bufferTuples;
	std::vector<std::tuple<const VkDescriptorImageInfo, const VkDescriptorType>>			   m_imageTuples;

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

		// Clear tuple vectors
		m_bufferTuples.clear();
		m_imageTuples.clear();

		// Allocate the descriptor sets
		if ( vkAllocateDescriptorSets( *m_logicalDevice, &descriptorSetAllocInfo, m_sets.data() ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to allocsate descriptor sets" );
	}

	void AddBufferSets( const std::vector<VkBuffer>& p_buffers, const VkDeviceSize& p_offset, const uint32_t& p_bufferSize, const VkDescriptorType& p_type )
	{
		// Create a vector of buffer info objects
		std::vector<VkDescriptorBufferInfo> bufferInfos {};

		// Fill vector with buffer info objects
		for ( uint32_t i = 0; i < m_size; i++ )
		{
			// Configure the descriptors using buffer information
			VkDescriptorBufferInfo bufferInfo {};
			bufferInfo.buffer = p_buffers[i];
			bufferInfo.offset = p_offset;
			bufferInfo.range  = p_bufferSize;

			// Add to buffer infos vector
			bufferInfos.push_back( bufferInfo );
		}

		// Add buffer information to the vector of buffers
		m_bufferTuples.push_back( std::make_tuple( bufferInfos, p_type ) );
	}

	void AddImageSets( const VkImageLayout& p_imageLayout, const VkImageView& p_imageView, const VkSampler& p_sampler, const VkDescriptorType& p_type )
	{
		// Configure the descriptors using image information
		VkDescriptorImageInfo imageInfo {};
		imageInfo.imageLayout = p_imageLayout;
		imageInfo.imageView	  = p_imageView;
		imageInfo.sampler	  = p_sampler;

		// Add image information to the vector of images
		m_imageTuples.push_back( std::make_tuple( imageInfo, p_type ) );
	}

	void UpdateSets()
	{
		// Create a writes vector
		std::vector<VkWriteDescriptorSet> writes {};

		// Iterate over all sets
		for ( uint32_t i = 0; i < m_size; i++ )
		{
			// Create a new write
			VkWriteDescriptorSet newWrite {};

			// Reset writes vector
			writes = {};

			// Add each buffer to the writes
			for ( const auto& bufferTuple : m_bufferTuples )
			{
				// Clear newWrite
				newWrite = {};

				// Configure the buffer write descriptor set
				newWrite.sType			  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				newWrite.dstSet			  = m_sets[i];
				newWrite.dstBinding		  = static_cast<uint32_t>( writes.size() ); // The index of this write
				newWrite.dstArrayElement  = 0;
				newWrite.descriptorType	  = std::get<1>( bufferTuple );
				newWrite.descriptorCount  = 1;
				newWrite.pBufferInfo	  = &std::get<0>( bufferTuple )[i];
				newWrite.pImageInfo		  = nullptr;
				newWrite.pTexelBufferView = nullptr;

				// Add newWrite to the writes vector
				writes.push_back( newWrite );
			}

			// Add each image to the writes
			for ( const auto& imageTuple : m_imageTuples )
			{
				// Clear newWrite
				newWrite = {};

				// Configure the image write descriptor set
				newWrite.sType			  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				newWrite.dstSet			  = m_sets[i];
				newWrite.dstBinding		  = static_cast<uint32_t>( writes.size() ); // The index of this write
				newWrite.dstArrayElement  = 0;
				newWrite.descriptorType	  = std::get<1>( imageTuple );
				newWrite.descriptorCount  = 1;
				newWrite.pBufferInfo	  = nullptr;
				newWrite.pImageInfo		  = &std::get<0>( imageTuple );
				newWrite.pTexelBufferView = nullptr;

				// Add newWrite to the writes vector
				writes.push_back( newWrite );
			}

			// Update the descriptor sets
			vkUpdateDescriptorSets( *m_logicalDevice, static_cast<uint32_t>( writes.size() ), writes.data(), 0, nullptr );
		}
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