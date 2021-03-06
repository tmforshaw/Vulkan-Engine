#version 460
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec2 inTexCoord;
layout( location = 3 ) in uint inSamplerID;

// clang-format off
layout( binding = 0 ) uniform VertexUniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 lightColour;
	vec3 lightPosition;
} ubo;
// clang-format on

layout( location = 0 ) out vec3 oFragPos;
layout( location = 1 ) out vec3 oFragNormal;
layout( location = 2 ) out vec2 oFragTexCoord;
layout( location = 3 ) out flat uint oFragSamplerID;
layout( location = 4 ) out vec3 outLightDir;

void main()
{
	// Position for the vertex shader output
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4( inPosition, 1.0 );

	// Ouput variables
	oFragPos	   = vec3( ubo.view * ubo.model * vec4( inPosition, 1.0 ) ); // Model matrix is pre-applied
	oFragTexCoord  = inTexCoord;
	oFragNormal	   = inNormal;
	oFragSamplerID = inSamplerID;
	outLightDir	   = normalize( vec3( ubo.view * vec4( inPosition - ubo.lightPosition, 1.0 ) ) );
	// outFragViewMat = ubo.view;
}