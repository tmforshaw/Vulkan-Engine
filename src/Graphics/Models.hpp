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
typedef uint32_t indexBufferType;

const std::string MODEL_PATH = "resources/models/viking_room.obj";

// Define the hash function for Vertex
namespace std
{
	template<>
	struct hash<Vertex>
	{
		size_t operator()( Vertex const& vertex ) const
		{
			return ( ( hash<glm::vec3>()( vertex.position ) ^
					   ( hash<glm::vec3>()( vertex.colour ) << 1 ) ) >>
					 1 ) ^
				   ( hash<glm::vec2>()( vertex.texCoord ) << 1 );
		}
	};
} // namespace std

void LoadModel( std::vector<Vertex>* p_vertices, std::vector<indexBufferType>* p_indices )
{
	tinyobj::attrib_t							attrib;
	std::vector<tinyobj::shape_t>				shapes;
	std::vector<tinyobj::material_t>			materials;
	std::string									warn, err;
	std::unordered_map<Vertex, indexBufferType> uniqueVertices {};

	if ( !tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str() ) )
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
				uniqueVertices[vertex] = static_cast<indexBufferType>( p_vertices->size() );
				p_vertices->push_back( vertex );
			}

			p_indices->push_back( uniqueVertices[vertex] );
		}
	}
}