#version 460
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inColour;

// clang-format off
layout( binding = 0 ) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;
// clang-format on

layout( location = 0 ) out vec3 oFragColour;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4( inPosition.xyz, 1.0 );
	oFragColour = inColour;
}