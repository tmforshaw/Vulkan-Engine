#pragma once

#include "./Models.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class WorldObject
{
private:
	Model	  m_model;
	glm::vec3 m_position;
	glm::quat m_rotation;
	glm::vec3 m_scale;

public:
	void Init( const char* modelPath, const char* texturePath, const glm::vec3& p_position, const glm::quat& p_rotation, const glm::vec3& p_scale, const VkSampleCountFlagBits& p_sampleCount, const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue,
			   const VkPhysicalDeviceProperties& p_physicalDeviceProperties, const VkFormat& p_format, const VkImageTiling& p_tiling,
			   const VkImageUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties,
			   const VkImageAspectFlags& p_aspectFlags, const uint32_t& p_samplerID )
	{
		// Set member variables
		m_position = p_position;
		m_rotation = p_rotation;
		m_scale	   = p_scale;

		// Initialise the model
		InitModel( modelPath, texturePath, p_sampleCount, p_logicalDevice, p_physicalDevice, p_commandPool, p_graphicsQueue, p_physicalDeviceProperties, p_format, p_tiling,
				   p_usage, p_properties, p_aspectFlags, p_samplerID );
	}

	void InitModel( const char* modelPath, const char* texturePath, const VkSampleCountFlagBits& p_sampleCount, const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue,
					const VkPhysicalDeviceProperties& p_physicalDeviceProperties, const VkFormat& p_format, const VkImageTiling& p_tiling,
					const VkImageUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties,
					const VkImageAspectFlags& p_aspectFlags, const uint32_t& p_samplerID )
	{
		// Initialise the model
		m_model.Init( modelPath, texturePath, p_sampleCount, p_logicalDevice, p_physicalDevice, p_commandPool, p_graphicsQueue, p_physicalDeviceProperties, p_format, p_tiling,
					  p_usage, p_properties, p_aspectFlags, p_samplerID );
	}

	void ApplyModelMatrix()
	{
		// Translate, rotate, and scale the vertices
		m_model.ApplyMatrix( glm::scale( glm::toMat4( m_rotation ) * glm::translate( glm::mat4( 1.0f ), m_position ), m_scale ) );
	}

	std::vector<Vertex> GetVerticesAfterModelMatrix() const
	{
		// Translate, rotate, and scale the vertices
		return m_model.GetVerticesAfterMatrix( glm::scale( glm::toMat4( m_rotation ) * glm::translate( glm::mat4( 1.0f ), m_position ), m_scale ) );
	}

	inline const Model&		GetModel() const { return m_model; }
	inline Model&			GetModelRef() { return m_model; }
	inline const glm::vec3& GetPos() const { return m_position; }
	inline const glm::quat& GetRot() const { return m_rotation; }
	inline const glm::vec3& GetScale() const { return m_scale; }

	inline void SetPos( const glm::vec3& p_position ) { m_position = p_position; }
	inline void SetRot( const glm::quat& p_rotation ) { m_rotation = p_rotation; }
	inline void SetScale( const glm::vec3& p_scale ) { m_scale = p_scale; }

	void Cleanup()
	{
		m_model.Cleanup();
	}
};