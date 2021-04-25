#define TINYOBJLOADER_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

#include "../Buffers/Vertex.hpp"
#include "../Graphics/Textures.hpp"

#include <glm/gtx/hash.hpp>
#include <stdexcept>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <vector>

// Define the type of int used in the indices vector
#define INDEX_BUFFER_TYPE VK_INDEX_TYPE_UINT32
typedef uint32_t IndexBufferType;

// Define the hash function for Vertex so it can be put into a hash table
namespace std
{
	template<>
	struct hash<Vertex>
	{
		size_t operator()( Vertex const& vertex ) const
		{
			return ( ( hash<glm::vec3>()( vertex.position ) ^ ( hash<glm::vec3>()( vertex.normal ) << 1 ) ) >> 1 ) ^
				   ( ( ( hash<glm::vec2>()( vertex.texCoord ) << 1 ) ^ ( hash<uint32_t>()( vertex.samplerID ) << 2 ) ) >> 2 );
		}
	};
} // namespace std

class Model
{
private:
	std::vector<Vertex>			 m_vertices;
	std::vector<IndexBufferType> m_indices;
	Texture						 m_texture;

public:
	void LoadModel( const char* path )
	{
		tinyobj::attrib_t							attrib;
		std::vector<tinyobj::shape_t>				shapes;
		std::vector<tinyobj::material_t>			materials;
		std::string									warn, err;
		std::unordered_map<Vertex, IndexBufferType> uniqueVertices {};

		if ( !tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, path ) )
			throw std::runtime_error( warn + err );

		// Reset vertices and indices
		m_vertices = {};
		m_indices  = {};

		// Combine all of the faces into a single model
		for ( const auto& shape : shapes )
		{
			for ( const auto& index : shape.mesh.indices )
			{
				Vertex vertex {};

				// Get position
				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				// Get the surface normals
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					-attrib.normals[3 * index.normal_index + 1], // Flip vertically
					attrib.normals[3 * index.normal_index + 2]
				};

				// Get texCoords and flip vertically
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				// Get the samplerID
				vertex.samplerID = m_texture.GetSamplerID();

				// Add to vertices if it is unique
				if ( uniqueVertices.count( vertex ) == 0 )
				{
					uniqueVertices[vertex] = static_cast<IndexBufferType>( m_vertices.size() );
					m_vertices.push_back( vertex );
				}

				m_indices.push_back( uniqueVertices[vertex] );
			}
		}
	}

	void LoadTexture( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue,
					  const VkPhysicalDeviceProperties& p_physicalDeviceProperties, const char* path, const VkSampleCountFlagBits& p_sampleCount, const VkFormat& p_format, const VkImageTiling& p_tiling,
					  const VkImageUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties,
					  const VkImageAspectFlags& p_aspectFlags, const uint32_t& p_samplerID )
	{
		m_texture.Init( p_logicalDevice, p_physicalDevice, p_commandPool, p_graphicsQueue, p_physicalDeviceProperties, path, p_sampleCount, p_format, p_tiling, p_usage, p_properties,
						p_aspectFlags, p_samplerID );
	}

	void Init( const char* modelPath, const char* texturePath, const VkSampleCountFlagBits& p_sampleCount, const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue,
			   const VkPhysicalDeviceProperties& p_physicalDeviceProperties, const VkFormat& p_format, const VkImageTiling& p_tiling,
			   const VkImageUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties,
			   const VkImageAspectFlags& p_aspectFlags, const uint32_t& p_samplerID )
	{
		// Reset vertices and indices
		m_vertices = {};
		m_indices  = {};

		// Load the texture
		LoadTexture( p_logicalDevice, p_physicalDevice, p_commandPool, p_graphicsQueue, p_physicalDeviceProperties, texturePath, p_sampleCount, p_format, p_tiling, p_usage, p_properties,
					 p_aspectFlags, p_samplerID );

		// Load the vertices and indices
		LoadModel( modelPath );
	}

	void TransitionTextureLayout( const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkImageLayout& p_oldLayout, const VkImageLayout& p_newLayout )
	{
		m_texture.TransitionLayout( p_commandPool, p_graphicsQueue, p_oldLayout, p_newLayout );
	}

	void SetVerticesAndIndices( const std::vector<Vertex>& p_vertices, const std::vector<IndexBufferType>& p_indices )
	{
		// Clear and resize the vertices vector
		m_vertices.clear();
		m_vertices.resize( p_vertices.size() );

		// Copy all vertices and change the sampler ID
		for ( uint32_t i = 0; i < p_vertices.size(); i++ )
		{
			m_vertices[i] = p_vertices[i];
			// m_vertices[i].normal.y	= -m_vertices[i].normal.y; // Flip vertically
			m_vertices[i].samplerID = m_texture.GetSamplerID();
		}

		m_indices = p_indices;
	}

	void ApplyMatrix( const glm::mat4& p_modelMatrix, const glm::mat3& p_normalMatrix )
	{
		// Iterate through the vertices and apply the matrix
		for ( uint32_t i = 0; i < m_vertices.size(); i++ )
		{
			m_vertices[i].position = glm::vec3( p_modelMatrix * glm::vec4( m_vertices[i].position, 1.0f ) );
			m_vertices[i].normal   = glm::vec3( p_normalMatrix * m_vertices[i].normal );
		}
	}

	std::vector<Vertex> GetVerticesAfterMatrix( const glm::mat4& p_modelMatrix, const glm::mat3& p_normalMatrix ) const
	{
		std::vector<Vertex> modifiedVertices = m_vertices;

		// Iterate through the modifiedVertices and apply the matrix
		for ( uint32_t i = 0; i < m_vertices.size(); i++ )
		{
			modifiedVertices[i].position = glm::vec3( p_modelMatrix * glm::vec4( m_vertices[i].position, 1.0f ) );
			modifiedVertices[i].normal	 = glm::vec3( p_normalMatrix * m_vertices[i].normal );
		}

		return modifiedVertices;
	}

	inline const std::vector<Vertex>&		   GetVertices() const { return m_vertices; }
	inline const std::vector<IndexBufferType>& GetIndices() const { return m_indices; }

	inline const std::vector<IndexBufferType> GetAdjustedIndices( const IndexBufferType& offset ) const
	{
		// Define a vector of indices to alter
		std::vector<IndexBufferType> adjustedIndices( m_indices.size() );

		// Add the offset to the indices
		for ( uint16_t i = 0; i < m_indices.size(); i++ )
			adjustedIndices[i] = m_indices[i] + offset;

		return adjustedIndices;
	}

	inline const Texture& GetTexture() const { return m_texture; }

	inline void Cleanup()
	{
		m_texture.Cleanup();
	}
};

// clang-format off
// const std::vector<Vertex> cubeVertices = {
// 	{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f } },
// 	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
// 	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
// 	{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
// 	{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
// 	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
// 	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
// 	{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } },
// 	{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
// 	{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } },
// 	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
// 	{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } },
// 	{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
// 	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } },
// 	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
// 	{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } }
// };


const std::vector<Vertex> cubeVertices = {
    { { -0.5f, -0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f }, },
    { {  0.5f, -0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f }, }, 
    { {  0.5f,  0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f }, }, 
    { {  0.5f,  0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f }, }, 
    { { -0.5f,  0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f }, }, 
    { { -0.5f, -0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f }, }, 

    { { -0.5f, -0.5f,  0.5f }, { 0.0f,  0.0f, 1.0f, }, { 0.0f, 1.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.0f,  0.0f, 1.0f, }, { 1.0f, 1.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.0f,  0.0f, 1.0f, }, { 1.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.0f,  0.0f, 1.0f, }, { 1.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.0f,  0.0f, 1.0f, }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.0f,  0.0f, 1.0f, }, { 0.0f, 1.0f } },

    { { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } },
    { { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },
    { { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },
    { { -0.5f, -0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },

    { {  0.5f,  0.5f,  0.5f }, { 1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },

    { { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } },

    { { -0.5f,  0.5f, -0.5f }, { 0.0f,  1.0f,  0.0f }, { 0.0f, 1.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.0f,  1.0f,  0.0f }, { 1.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.0f,  1.0f,  0.0f }, { 1.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.0f,  1.0f,  0.0f }, { 0.0f, 1.0f } }
};
// clang-format on

// const std::vector<IndexBufferType> cubeIndices = {
// 	0, 2, 1, 2, 0, 3,	  // Front Face
// 	4, 5, 6, 6, 7, 4,	  // Back Face
// 	9, 10, 11, 11, 4, 9,  // Right Face
// 	13, 15, 2, 15, 13, 8, // Left Face
// 	11, 14, 5, 5, 4, 11,  // Bottom Face
// 	3, 13, 2, 13, 3, 12	  // Top Face
// };

const std::vector<IndexBufferType> cubeIndices = {
	0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35
};