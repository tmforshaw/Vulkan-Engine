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
			return ( ( hash<glm::vec3>()( vertex.position ) ^ ( hash<glm::vec3>()( vertex.colour ) << 1 ) ) >> 1 ) ^
				   ( hash<glm::vec2>()( vertex.texCoord ) << 1 );
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

		// Combine all of the faces into a single model
		for ( const auto& shape : shapes )
		{
			for ( const auto& index : shape.mesh.indices )
			{
				Vertex vertex {};

				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				// Flip vertically
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.colour = { 1.0f, 1.0f, 1.0f };

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
					  const VkImageAspectFlags& p_aspectFlags )
	{
		m_texture.Init( p_logicalDevice, p_physicalDevice, p_commandPool, p_graphicsQueue, p_physicalDeviceProperties, path, p_sampleCount, p_format, p_tiling, p_usage, p_properties,
						p_aspectFlags );
	}

	void Init( const char* modelPath, const char* texturePath, const VkSampleCountFlagBits& p_sampleCount, const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue,
			   const VkPhysicalDeviceProperties& p_physicalDeviceProperties, const VkFormat& p_format, const VkImageTiling& p_tiling,
			   const VkImageUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties,
			   const VkImageAspectFlags& p_aspectFlags )
	{

		// Load the texture
		LoadTexture( p_logicalDevice, p_physicalDevice, p_commandPool, p_graphicsQueue, p_physicalDeviceProperties, texturePath, p_sampleCount, p_format, p_tiling, p_usage, p_properties,
					 p_aspectFlags );

		// Load the vertices and indices
		LoadModel( modelPath );
	}

	void TransitionTextureLayout( const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkImageLayout& p_oldLayout, const VkImageLayout& p_newLayout )
	{
		m_texture.TransitionLayout( p_commandPool, p_graphicsQueue, p_oldLayout, p_newLayout );
	}

	void SetVerticesAndIndices( const std::vector<Vertex>& p_vertices, const std::vector<IndexBufferType>& p_indices )
	{
		m_vertices = p_vertices;
		m_indices  = p_indices;
	}

	inline const std::vector<Vertex>&		   GetVertices() const { return m_vertices; }
	inline const std::vector<IndexBufferType>& GetIndices() const { return m_indices; }

	inline const Texture& GetTexture() const { return m_texture; }

	inline void Cleanup()
	{
		m_texture.Cleanup();
	}
};

// clang-format off
const std::vector<Vertex> cubeVertices = {
	{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
	{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
	{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
	{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
	{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } }
};
// clang-format on

const std::vector<IndexBufferType> cubeIndices = {
	0, 2, 1, 2, 0, 3,	  // Front Face
	4, 5, 6, 6, 7, 4,	  // Back Face
	9, 10, 11, 11, 4, 9,  // Right Face
	13, 15, 2, 15, 13, 8, // Left Face
	11, 14, 5, 5, 4, 11,  // Bottom Face
	3, 13, 2, 13, 3, 12	  // Top Face
};