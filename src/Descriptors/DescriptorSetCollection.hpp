#pragma once

#include "DescriptorSetLayout.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

// class DescriptorSet
// {
// private:
// 	VkDescriptorSet					  m_set;
// 	std::vector<VkWriteDescriptorSet> m_writes;

// public:
// 	void AddWrite( const VkDescriptorType& p_type, const uint32_t p_descriptorCount, const VkDescriptorBufferInfo* p_bufferInfo, const VkDescriptorImageInfo* p_imageInfo, const VkBufferView* p_texelView )
// 	{
// 		// Create a write descriptor set
// 		VkWriteDescriptorSet write;

// 		// Configure the write descriptor set
// 		write.sType			   = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// 		write.dstSet		   = m_set;
// 		write.dstBinding	   = m_writes.size(); // Set binding to the size of the writes vector
// 		write.dstArrayElement  = 0;
// 		write.descriptorType   = p_type;
// 		write.descriptorCount  = p_descriptorCount;
// 		write.pBufferInfo	   = p_bufferInfo;
// 		write.pImageInfo	   = p_imageInfo;
// 		write.pTexelBufferView = p_texelView;

// 		// Add the write onto the writes vector
// 		m_writes.push_back( write );
// 	}

// 	inline const VkDescriptorSet& GetSet() const { return m_set; }

// 	void Update( const VkDevice& p_logicalDevice )
// 	{
// 		// Update the descriptor sets
// 		vkUpdateDescriptorSets( p_logicalDevice, static_cast<uint32_t>( m_writes.size() ), m_writes.data(), 0, nullptr );
// 	}
// };

class DescriptorSetCollection
{
private:
	// std::vector<DescriptorSet> m_sets;
	std::vector<VkDescriptorSet>				   m_sets;
	std::vector<std::vector<VkWriteDescriptorSet>> m_writes;

	// std::vector<std::vector<VkWriteDescriptorSet>> m_writes;

	uint32_t		m_writeIndex;
	const VkDevice* m_logicalDevice;

public:
	void InitSets( const VkDevice& p_logicalDevice, const uint32_t& p_size, const VkDescriptorSetLayout& p_layout, const VkDescriptorPool& p_pool )
	{
		// Set member variables
		m_logicalDevice = const_cast<VkDevice*>( &p_logicalDevice );
		m_writeIndex	= 0;

		// Create a vector of of descriptor set layouts and fill the layout parameter
		std::vector<VkDescriptorSetLayout> layouts( p_size, p_layout );

		// Setup the allocation information for the descriptor sets
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo {};
		descriptorSetAllocInfo.sType			  = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.descriptorPool	  = p_pool;
		descriptorSetAllocInfo.descriptorSetCount = p_size;
		descriptorSetAllocInfo.pSetLayouts		  = layouts.data();

		// Resize the descriptor set vector
		m_sets.resize( p_size );
		m_writes.resize( p_size );

		// Allocate the descriptor sets
		if ( vkAllocateDescriptorSets( *m_logicalDevice, &descriptorSetAllocInfo, m_sets.data() ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to allocate descriptor sets" );
	}

	void AddWrite( const uint32_t& p_index, const VkDescriptorType& p_type, const uint32_t p_descriptorCount, const VkDescriptorBufferInfo* p_bufferInfo, const VkDescriptorImageInfo* p_imageInfo, const VkBufferView* p_texelView )
	{
		// Create a write descriptor set
		VkWriteDescriptorSet write;

		// Configure the write descriptor set
		write.sType			   = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet		   = m_sets[p_index];
		write.dstBinding	   = m_writeIndex++; // Increment the write index afterwards
		write.dstArrayElement  = 0;
		write.descriptorType   = p_type;
		write.descriptorCount  = p_descriptorCount;
		write.pBufferInfo	   = p_bufferInfo;
		write.pImageInfo	   = p_imageInfo;
		write.pTexelBufferView = p_texelView;

		// Add the write onto the writes vector
		m_writes[p_index].push_back( write );
	}

	void UpdateSet( const uint32_t& p_index )
	{
		// Update the descriptor sets
		vkUpdateDescriptorSets( *m_logicalDevice, static_cast<uint32_t>( m_writes[p_index].size() ), m_writes[p_index].data(), 0, nullptr );

		// Reset the write index
		m_writeIndex = 0;
	}

	inline const VkDescriptorSet& GetSet( const uint32_t& i ) const { return m_sets[i]; }

	// inline const std::vector<VkDescriptorSet> GetSets() const
	// {
	// 	std::vector<VkDescriptorSet> rawSets( m_sets.size() );
	// 	for ( uint32_t i = 0; i < m_sets.size(); i++ )
	// 		rawSets[i] = m_sets[i].GetSet();

	// 	return rawSets;
	// }

	void Cleanup()
	{
	}
};