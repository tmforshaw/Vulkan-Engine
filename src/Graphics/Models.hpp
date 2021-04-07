#define TINYOBJLOADER_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

#include "../Buffers/Vertex.hpp"

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

public:
	void LoadFromFile( const char* path )
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

	inline const std::vector<Vertex>&		   GetVertices() const { return m_vertices; }
	inline const std::vector<IndexBufferType>& GetIndices() const { return m_indices; }
};